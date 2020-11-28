#include <unistd.h>
#include <algorithm> // for sort
#include <chrono>
#include <random>
#include <fstream>
#include <sys/types.h>   // for thread id
#include <sys/syscall.h> // for thread id
#include <stdsc/stdsc_log.hpp>
#include <omp.h>
#include <ppcnn_server/ppcnn_server_query.hpp>
#include <ppcnn_server/ppcnn_server_result.hpp>
#include <ppcnn_server/ppcnn_server_calcthread.hpp>
#include <seal/seal.h>

namespace ppcnn_server
{

struct CalcThread::Impl
{
    Impl(QueryQueue& in_queue,
         ResultQueue& out_queue)
        : in_queue_(in_queue),
          out_queue_(out_queue)
    {
    }

    void exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        auto th_id = syscall(SYS_gettid);
        STDSC_LOG_INFO("Launched calcuration thread. (thread ID: %d)", th_id);
        
        while (!args.force_finish) {

            STDSC_LOG_INFO("[th:%d] Try getting query from QueryQueue.", th_id);

            int32_t query_id;
            Query query;
            while (!in_queue_.pop(query_id, query)) {
                usleep(args.retry_interval_msec * 1000);
            }

            bool status = true;
            
            STDSC_LOG_INFO("[th:%d] Get query #%d.", th_id, query_id);

            seal::Ciphertext dummy_result;
            Result result(query_id, status, dummy_result);
            out_queue_.push(query_id, result);

            STDSC_LOG_INFO("[th:%d] Set result of query #%d.", th_id, query_id);
        }
    }

    QueryQueue& in_queue_;
    ResultQueue& out_queue_;
    CalcThreadParam param_;
    std::shared_ptr<stdsc::ThreadException> te_;
};

CalcThread::CalcThread(QueryQueue& in_queue,
                       ResultQueue& out_queue)
    : pimpl_(new Impl(in_queue, out_queue))
{}

void CalcThread::start()
{
    pimpl_->param_.force_finish = false;
    super::start(pimpl_->param_, pimpl_->te_);
}

void CalcThread::stop()
{
    STDSC_LOG_INFO("Stop calculation thread.");
    pimpl_->param_.force_finish = true;
}

void CalcThread::exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te) const
{
    pimpl_->exec(args, te);
}

} /* namespace ppcnn_server */
