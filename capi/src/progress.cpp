#include "scid/libscid/progress.h"

namespace scid::libscid
{

    CallbackProgress::CallbackProgress(
        scid_progress_report_callback progress_report,
        void*                         progress_report_user_data,
        scid_should_cancel_fn         should_cancel,
        void*                         should_cancel_user_data)
        : progress_report_(progress_report),
          progress_report_user_data_(progress_report_user_data),
          should_cancel_(should_cancel),
          should_cancel_user_data_(should_cancel_user_data)
    {}


    bool
    CallbackProgress::report(
        size_t      done,
        size_t      total,
        const char* message)
    {
        if (progress_report_)
        {
            progress_report_(done, total, message, progress_report_user_data_);
        }
        return !should_cancel_ || !should_cancel_(should_cancel_user_data_);
    }

} // namespace scid::libscid
