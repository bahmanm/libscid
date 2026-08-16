#ifndef SCID_LIBSCID_PROGRESS_H
#define SCID_LIBSCID_PROGRESS_H

#include "scid/database.h"

#include "scid/database/misc.h"

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

} // namespace scid::libscid

#endif
