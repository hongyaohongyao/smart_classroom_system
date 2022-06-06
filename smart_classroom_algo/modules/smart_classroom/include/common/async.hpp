
#ifndef SMART_CLASSROOM_CPP_ASYNC_HPP
#define SMART_CLASSROOM_CPP_ASYNC_HPP

#include "future"
#include <common/common_utils.hpp>
/************\
 异步处理相关
\***********/
namespace smc::async_utils {
    /**
     * 创建一个异步执行对象，commit参数后返回输出类型的shared_future对象。提交的任务由单个线程顺序执行。
     * @tparam Output 输出类型
     * @tparam Args 输入参数
     */
    template<class Output, class ...Args>
    class SingletonAsync {

    public:
        SingletonAsync() {
            run_ = true;
            worker_ = std::make_shared<std::thread>(&SingletonAsync::worker, this);
        }

        ~SingletonAsync() {
            stop();
        }

        std::shared_future<Output> commit(const Args &... inputs) {
            Job job;
            job.pro = std::make_shared<std::promise<Output>>();
            job.args = std::make_tuple(inputs...);
            std::unique_lock<std::mutex> l(jobs_lock_);
            jobs_.push(job);
            cond_.notify_one();
            return job.pro->get_future();
        }


    protected:
        struct Job {
            std::tuple<Args ...> args;
            std::shared_ptr<std::promise<Output>> pro;
        };


        virtual Output process(const Args &...args) = 0;

        void stop() {
            run_ = false;
            cond_.notify_all();

            ////////////////////////////////////////// cleanup jobs
            {
                std::unique_lock<std::mutex> l(jobs_lock_);
                while (!jobs_.empty()) {
                    auto &item = jobs_.front();
                    if (item.pro)
                        item.pro->set_value(Output());
                    jobs_.pop();
                }
            };

            if (worker_) {
                worker_->join();
                worker_.reset();
            }
        }

        void worker() {
            realWorker(typename common_utils::tuple_bind<sizeof ...(Args)>::type());
        }

        template<size_t ...Is>
        void realWorker(common_utils::tuple_idx<Is...> &&) {
            Job job;
            while (get_job_or_wait(job)) {
                job.pro->set_value(process(std::get<Is>(job.args)...));
            }
        }

        bool get_job_or_wait(Job &fetch_job) {
            std::unique_lock<std::mutex> l(jobs_lock_);
            cond_.wait(l, [&]() {
                return !run_ || !jobs_.empty();
            });

            if (!run_) return false;

            fetch_job = std::move(jobs_.front());
            jobs_.pop();
            return true;
        }

        std::atomic<bool> run_{};
        std::mutex jobs_lock_;
        std::queue<Job> jobs_{};
        std::shared_ptr<std::thread> worker_;
        std::condition_variable cond_;
    };

    /**
         * 一个单线程的线程池。
         * @tparam Output 输出类型
         * @tparam Args 输入参数
         */
    class SingletonThreadPool {
    public:
        SingletonThreadPool() {
            run_ = true;
            worker_ = std::make_shared<std::thread>(&SingletonThreadPool::worker, this);
        }

        ~SingletonThreadPool() {
            stop();
        }


        template<class Fn, class... Args>
        auto commit(Fn &&fn, Args &&... args) -> std::shared_future<typename std::result_of<Fn(Args...)>::type> {
            using return_type = typename std::result_of<Fn(Args...)>::type;
            auto task = std::make_shared<std::packaged_task<return_type()> >(
                    std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...)
            );

            auto result = task->get_future();
            {
                std::unique_lock<std::mutex> l(jobs_lock_);
                jobs_.emplace([task]() { (*task)(); });
                cond_.notify_one();
            }
            return result;
        }


    protected:

        void stop() {
            run_ = false;
            cond_.notify_all();

            ////////////////////////////////////////// cleanup jobs
            {
                std::unique_lock<std::mutex> l(jobs_lock_);
                while (!jobs_.empty()) {
                    jobs_.front()();
                    jobs_.pop();
                }
            };

            if (worker_) {
                worker_->join();
                worker_.reset();
            }
        }

        void worker() {
            std::function < void() > job;
            while (get_job_or_wait(job)) {
                job();
            }
        }

        bool get_job_or_wait(std::function<void()> &fetch_job) {
            std::unique_lock<std::mutex> l(jobs_lock_);
            cond_.wait(l, [&]() {
                return !run_ || !jobs_.empty();
            });

            if (!run_) return false;

            fetch_job = std::move(jobs_.front());
            jobs_.pop();
            return true;
        }

        std::atomic<bool> run_{};
        std::mutex jobs_lock_;
        std::queue<std::function<void()> > jobs_{};
        std::shared_ptr<std::thread> worker_;
        std::condition_variable cond_;
    };

    /**
    * 一个单线程顺序执行任务的对象。
    * @tparam Output 输出类型
    * @tparam Args 输入参数
    */
    template<class ...Args>
    class SingletonThread {

    public:
        SingletonThread() {
            run_ = true;
            worker_ = std::make_shared<std::thread>(&SingletonThread::worker, this);
        }

        ~SingletonThread() {
            stop();
        }

        void commit(const Args &... inputs) {
            Job job;
            job.args = std::make_tuple(inputs...);
            std::unique_lock<std::mutex> l(jobs_lock_);
            jobs_.push(job);
            cond_.notify_one();
        }


    protected:
        struct Job {
            std::tuple<Args ...> args;
        };


        virtual void process(const Args &...args) = 0;

        void stop() {
            run_ = false;
            cond_.notify_all();

            ////////////////////////////////////////// cleanup jobs
            {
                std::unique_lock<std::mutex> l(jobs_lock_);
                while (!jobs_.empty()) {
                    jobs_.pop();
                }
            };

            if (worker_) {
                worker_->join();
                worker_.reset();
            }
        }

        void worker() {
            realWorker(typename common_utils::tuple_bind<sizeof ...(Args)>::type());
        }

        template<size_t ...Is>
        void realWorker(common_utils::tuple_idx<Is...> &&) {
            Job job;
            while (get_job_or_wait(job)) {
                process(std::get<Is>(job.args)...);
            }
        }

        bool get_job_or_wait(Job &fetch_job) {
            std::unique_lock<std::mutex> l(jobs_lock_);
            cond_.wait(l, [&]() {
                return !run_ || !jobs_.empty();
            });

            if (!run_) return false;

            fetch_job = std::move(jobs_.front());
            jobs_.pop();
            return true;
        }

        std::atomic<bool> run_{};
        std::mutex jobs_lock_;
        std::queue<Job> jobs_{};
        std::shared_ptr<std::thread> worker_;
        std::condition_variable cond_;
    };
}
#endif //SMART_CLASSROOM_CPP_ASYNC_HPP
