package org.jeecg.utils;

import org.jeecg.common.api.vo.Result;

public class ResultUtils {
    public static Result<?> msgResult(String errMsg, String okMsg) {
        return errMsg == null ? Result.OK(okMsg) : Result.error(errMsg);
    }
}
