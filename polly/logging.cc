// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#include "rctems/common/logging.h"

#include <utility>

#include "boost/log/support/date_time.hpp"
#include "boost/log/attributes.hpp"
#include "boost/log/common.hpp"
#include "boost/log/core.hpp"
#include "boost/log/exceptions.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/sinks.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/utility/setup.hpp"

// BOOST_LOG_ATTRIBUTE_KEYWORD(file, "RCT_FILE", const char*);
// BOOST_LOG_ATTRIBUTE_KEYWORD(line, "RCT_LINE", int16_t);

namespace rctems {

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace {
inline logging::trivial::severity_level ToEnum(const std::string& severity) {
    return severity == "trace" ? logging::trivial::trace
            : severity == "debug" ? logging::trivial::debug
            : severity == "info" ? logging::trivial::info
            : severity == "warning" ? logging::trivial::warning
            : severity == "error" ? logging::trivial::error
            : severity == "fatal" ? logging::trivial::fatal
            : logging::trivial::warning;
}

#define TS(fmt) \
    expr::format_date_time<boost::posix_time::ptime>("Timestamp", fmt)

}  // anonymous namespace

void InitLogSystem(const std::string& severity, const std::string& sinks) {
    auto core = boost::log::core::get();
    logging::add_common_attributes();
    // core->add_global_attribute(
    //         "RCT_FILE",
    //         attrs::mutable_constant<const char*>(""));
    // core->add_global_attribute(
    //         "RCT_LINE",
    //         attrs::constant<int>(0));

    const char* fmt = "%TimeStamp% %Severity% %Message%";

    core->remove_all_sinks();
    if (sinks == "console") {
        logging::add_console_log(std::clog, keywords::format = fmt);
    }
    if (sinks == "file") {
        auto sink = logging::add_file_log(
                keywords::file_name = "rctems_%N.log",
                keywords::rotation_size = 10 * 1024 * 1024,
                keywords::time_based_rotation =
                    sinks::file::rotation_at_time_point(0, 0, 0),
                keywords::auto_flush = true,
                keywords::target = ".",
                keywords::max_files = 10,
                keywords::format = fmt);
        sink->locked_backend()->scan_for_files();
    }

    core->set_filter(logging::trivial::severity >= ToEnum(severity));
}

}  // namespace rctems
