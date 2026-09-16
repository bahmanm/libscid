#ifndef SCID_LIBSCID_PROGRESS_H
#define SCID_LIBSCID_PROGRESS_H

#include "scid/database.h"

#include "scid/database/misc.h"

#include <memory>

namespace scid::libscid
{

    class CallbackProgress final : public scid::database::Progress::Impl
    {
            scid_progress_report_callback progress_report_;
            void*                         progress_report_user_data_;
            scid_should_cancel_fn         should_cancel_;
            void*                         should_cancel_user_data_;

        public:
            CallbackProgress(
                scid_progress_report_callback progress_report,
                void*                         progress_report_user_data,
                scid_should_cancel_fn         should_cancel,
                void*                         should_cancel_user_data);

            bool
            report(
                size_t      done,
                size_t      total,
                const char* message) final;
    };

    inline scid::database::Progress
    make_callback_progress(
        scid_progress_report_callback progress_report,
        void*                         progress_report_user_data,
        scid_should_cancel_fn         should_cancel,
        void*                         should_cancel_user_data)
    {
        if (progress_report == nullptr && should_cancel == nullptr)
        {
            return scid::database::Progress();
        }

        auto impl = std::make_unique<CallbackProgress>(
            progress_report, progress_report_user_data, should_cancel, should_cancel_user_data);
        return scid::database::Progress(impl.release());
    }

} // namespace scid::libscid

#endif
