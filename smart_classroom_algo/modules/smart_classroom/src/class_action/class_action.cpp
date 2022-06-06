#include "class_action/class_action.hpp"
#include <atomic>
#include <mutex>
#include <numeric>
#include <queue>
#include <infer/trt_infer.hpp>
#include <common/ilogger.hpp>
#include <common/infer_controller.hpp>
#include <common/preprocess_kernel.cuh>

namespace smc {
    namespace cls_act {
        void AffineMatrix::compute(const cv::Size &image_size, const cv::Rect &box, const cv::Size &net_size) {
            cv::Rect box_ = box;
            if (box_.width == 0 || box_.height == 0) {
                box_.width = image_size.width;
                box_.height = image_size.height;
                box_.x = 0;
                box_.y = 0;
            }

            float rate = box_.width > 100 ? 0.1f : 0.15f;
            float pad_width = (float) box_.width * (1 + 2 * rate);
            float pad_height = (float) box_.height * (1 + 1 * rate);
            float scale = std::min((float) net_size.width / pad_width, (float) net_size.height / pad_height);
            i2d[0] = scale;
            i2d[1] = 0;
            i2d[2] = (float) (-((float) box_.x - (float) box_.width * 1 * rate + pad_width * 0.5) * scale +
                              net_size.width * 0.5 +
                              scale * 0.5 -
                              0.5);
            i2d[3] = 0;
            i2d[4] = scale;
            i2d[5] = (float) (-((float) box_.y - (float) box_.height * 1 * rate + pad_height * 0.5) * scale +
                              net_size.height * 0.5 +
                              scale * 0.5 - 0.5);

            cv::Mat m2x3_i2d(2, 3, CV_32F, i2d);
            cv::Mat m2x3_d2i(2, 3, CV_32F, d2i);
            cv::invertAffineTransform(m2x3_i2d, m2x3_d2i);
        }

        cv::Mat AffineMatrix::i2d_mat() {
            return {2, 3, CV_32F, i2d};
        }

        static std::tuple<float, float> affine_project(float x, float y, float *pmatrix) {

            float newx = x * pmatrix[0] + y * pmatrix[1] + pmatrix[2];
            float newy = x * pmatrix[3] + y * pmatrix[4] + pmatrix[5];
            return std::make_tuple(newx, newy);
        }

        using ControllerImpl = InferController
                <
                        Input,                     // input
                        PoseAction,           // output
                        std::tuple<std::string, int>,        // start param
                        AffineMatrix               // additional
                >;

        class InferImpl : public Infer, public ControllerImpl {
        public:
            /** 要求在InferImpl里面执行stop，而不是在基类执行stop **/
            ~InferImpl() override {
                TRT::set_device(gpu_);
                stop();
            }

            bool startup(const std::string &file, int gpuid) {
                return ControllerImpl::startup(make_tuple(file, gpuid));
            }

            void worker(std::promise<bool> &result) override {

                std::string file = std::get<0>(start_param_);
                int gpuid = std::get<1>(start_param_);

                TRT::set_device(gpuid);
                auto engine = TRT::load_infer(file);
                if (engine == nullptr) {
                    INFOE("Engine %s load failed", file.c_str());
                    result.set_value(false);
                    return;
                }

                engine->print();

                int max_batch_size = engine->get_max_batch_size();
                auto input = engine->input();
                auto output = engine->output(0);
                auto output_cls_probs = engine->output(1);
                auto rawBestPreds = engine->output(2);
                auto bestPreds = engine->output(3);
                auto isPassing = engine->output(4);
                float stride = 4.0f;
                input_width_ = input->width();
                input_height_ = input->height();
                gpu_ = gpuid;
                tensor_allocator_ = std::make_shared<MonopolyAllocator<TRT::Tensor>>(max_batch_size * 5); //
                stream_ = engine->get_stream();
                result.set_value(true);
                input->resize_single_dim(0, max_batch_size);

                std::vector<Job> fetch_jobs;
                while (get_jobs_and_wait(fetch_jobs, max_batch_size)) {

                    int infer_batch_size = (int) fetch_jobs.size();
                    input->resize_single_dim(0, infer_batch_size);

                    for (int ibatch = 0; ibatch < infer_batch_size; ++ibatch) {
                        auto &job = fetch_jobs[ibatch];
                        input->copy_from_gpu(input->offset(ibatch), job.mono_tensor->data()->gpu(), input->count(1));
                        job.mono_tensor->release();
                    }

                    engine->forward(false);

                    for (int ibatch = 0; ibatch < infer_batch_size; ++ibatch) {
                        /**处理关键点**/
                        auto &job = fetch_jobs[ibatch];
                        auto *image_based_output = output->cpu<float>(ibatch);
                        auto &image_based_keypoints = job.output.pose;
                        auto &affine_matrix = job.additional;
                        int begin_channel = 0;
                        int area = output->height();
                        image_based_keypoints.resize(output->channel() - begin_channel);

                        for (int i = begin_channel; i < output->channel(); ++i) {
                            float *output_channel = image_based_output + i * area;

                            float x = output_channel[0];
                            float y = output_channel[1];
                            float confidence = output_channel[2];
                            x = x * stride;
                            y = y * stride;
                            auto &output_point = image_based_keypoints[i - begin_channel];

                            output_point.z = confidence;
                            std::tie(output_point.x, output_point.y) = affine_project(x, y, affine_matrix.d2i);
                        }
                        //                    for (int i = 26; i < 94; i++) {
                        //                        float *output_channel = image_based_output + i * area;
                        //                        cout << "(" << output_channel[0] << "," << output_channel[1] << ") ";
                        //                    }
                        //                    cout << endl;

                        /**处理动作分类**/
                        {
                            auto *cls_probs = output_cls_probs->cpu<float>(ibatch);
                            memcpy(job.output.actionProbs, cls_probs, sizeof job.output.actionProbs);
                        }
                        /**最佳课堂分类，最佳作弊动作分类，以及是否伸手**/
                        {
                            job.output.rawBestPred = *(rawBestPreds->cpu<int>(ibatch));
                            job.output.bestPred = *(bestPreds->cpu<int>(ibatch));
                            job.output.isPassing = *(isPassing->cpu<int>(ibatch));
                        }
                        /**输出结果**/
                        job.pro->set_value(job.output);
                    }


                    fetch_jobs.clear();
                }
                stream_ = nullptr;
                tensor_allocator_.reset();
                INFO("Engine destroy.");
            }

            std::shared_future<PoseAction> commit(const Input &input) override {
                return ControllerImpl::commit(input);
            }

            std::vector<std::shared_future<PoseAction>> commits(const std::vector<Input> &inputs) override {
                return ControllerImpl::commits(inputs);
            }

            bool preprocess(Job &job, const Input &input) override {

                if (tensor_allocator_ == nullptr) {
                    INFOE("tensor_allocator_ is nullptr");
                    return false;
                }

                job.mono_tensor = tensor_allocator_->query();
                if (job.mono_tensor == nullptr) {
                    INFOE("Tensor allocator query failed.");
                    return false;
                }

                CUDATools::AutoDevice auto_device(gpu_);
                auto &tensor = job.mono_tensor->data();
                if (tensor == nullptr) {
                    // not init
                    tensor = std::make_shared<TRT::Tensor>();
                    tensor->set_workspace(std::make_shared<TRT::MixMemory>());
                }

                auto &image = std::get<0>(input);
                auto &box = std::get<1>(input);
                cv::Size input_size(input_width_, input_height_);
                job.additional.compute(image.size(), box, input_size);

                tensor->set_stream(stream_);
                tensor->resize(1, 3, input_height_, input_width_);
                float mean[] = {0.406, 0.457, 0.480};
                float std[] = {1, 1, 1};

                size_t size_image = image.cols * image.rows * 3;
                size_t size_matrix = iLogger::upbound(sizeof(job.additional.d2i), 32);
                auto workspace = tensor->get_workspace();
                auto *gpu_workspace = (uint8_t *) workspace->gpu(size_image + size_matrix);
                auto *affine_matrix_device = (float *) gpu_workspace;
                uint8_t *image_device = gpu_workspace + size_matrix;
                checkCudaRuntime(
                        cudaMemcpyAsync(image_device, image.data, size_image, cudaMemcpyHostToDevice, stream_));
                checkCudaRuntime(cudaMemcpyAsync(affine_matrix_device, job.additional.d2i, sizeof(job.additional.d2i),
                                                 cudaMemcpyHostToDevice, stream_));

                auto normalize = CUDAKernel::Norm::mean_std(mean, std, 1 / 255.0f, CUDAKernel::ChannelType::Invert);
                CUDAKernel::warp_affine_bilinear_and_normalize_plane(
                        image_device, image.cols * 3, image.cols, image.rows,
                        tensor->gpu<float>(), input_width_, input_height_,
                        affine_matrix_device, 127,
                        normalize, stream_
                );
                return true;
            }

        private:
            int input_width_ = 0;
            int input_height_ = 0;
            int gpu_ = 0;
            TRT::CUStream stream_ = nullptr;
        };

        std::shared_ptr<Infer> create_infer(const std::string &engine_file, int gpuid) {
            std::shared_ptr<InferImpl> instance(new InferImpl());
            if (!instance->startup(engine_file, gpuid)) {
                instance.reset();
            }
            return instance;
        }
    }
}