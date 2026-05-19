#include <questdb/ingress/line_sender.hpp>
#include <iostream>

using namespace std::literals::string_view_literals;
using namespace questdb::ingress::literals;

std::string host = "";
std::string port = "";

static void setQuestDbHost(std::string_view newHost, std::string_view newPort){
    host = newHost;
    port = newPort;
}

static bool example(std::string quest_table_name)
{
    try
    {
        auto sender = questdb::ingress::line_sender::from_conf(
            "tcp::addr=" + std::string{host} + ":" + std::string{port} +
            ";protocol_version=2;");

        // We prepare all our table names and column names in advance.
        // If we're inserting multiple rows, this allows us to avoid
        // re-validating the same strings over and over again.
        const auto device_id = "device_id"_cn;
        const auto temperature = "price"_cn;
        const auto light = "amount"_cn;

        questdb::ingress::line_sender_buffer buffer = sender.new_buffer();
        buffer.table(quest_table_name)
            .symbol(device_id, "88:88:88:88"_utf8)
            .column(temperature, 26.5)
            .column(light, 430.3)
            .at(questdb::ingress::timestamp_nanos::now());

        // To insert more records, call `buffer.table(..)...` again.

        sender.flush(buffer);

        // It's recommended to keep a timer and/or maximum buffer size to flush
        // the buffer periodically with any accumulated records.

        return true;
    }
    catch (const questdb::ingress::line_sender_error& err)
    {
        std::cerr << "Error running example: " << err.what() << std::endl;

        return false;
    }
}

static bool displayed_help(int argc, const char* argv[])
{
    for (int index = 1; index < argc; ++index)
    {
        const std::string_view arg{argv[index]};
        if ((arg == "-h"sv) || (arg == "--help"sv))
        {
            std::cerr << "Usage:\n"
                      << "line_sender_c_example: [HOST [PORT]]\n"
                      << "    HOST: ILP host (defaults to \"localhost\").\n"
                      << "    PORT: ILP port (defaults to \"9000\")."
                      << std::endl;
            return true;
        }
    }
    return false;
}

int main(int argc, const char* argv[])
{
    setQuestDbHost("localhost","9000");
    if (displayed_help(argc, argv))
        return 0;

    auto host = "localhost"sv;
    if (argc >= 2)
        host = std::string_view{argv[1]};
    auto port = "9009"sv;
    if (argc >= 3)
        port = std::string_view{argv[2]};

    return !example("this should be the device name + \"table\"");
}