#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>


namespace transport_directory::input_reader{
    /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
    geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return {nan, nan};
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return {lat, lng};
    }

    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return {std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1))};
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    size_t FindSecondComma(std::string_view text) noexcept {
        size_t first_comma = text.find_first_of(',');
        size_t second_comma = text.find_first_of(',', first_comma+1);
        return second_comma;
    }

    void ParseRealDistances(std::string_view stopname, std::string_view text,
                            TransportCatalogue& catalogue)
    {
        auto comma = FindSecondComma(text);

        while (comma != text.npos){
            size_t num_start = text.find_first_not_of(' ', comma + 1);
            size_t num_end = text.find_first_of('m', num_start);
            const int num = std::move(
                std::stoi(std::string(text.substr(num_start, num_end - num_start + 1)))
            );

            size_t value_start = text.find_first_not_of(' ', text.find_first_of('t', num_end) + 2);
            comma = text.find_first_of(',', value_start + 1);
            size_t value_end = comma == text.npos ? text.size() : comma;
            const std::string_view stop = Trim(text.substr(value_start, value_end - value_start));

            catalogue.AddRealDistance(stopname, num, stop);
        }

    }

    const std::pair<command_pointers, command_pointers> InputReader::SeparateStopsAndBuses() const {
        using namespace std::literals;

        command_pointers stops, buses;

        for (const auto& command : commands_){
            if (!command){
                continue;
            }
            if (command.command == "Stop"s){
                const CommandDescription *c = &command;
                stops.push_back(c);
            } else if (command.command == "Bus"s){
                const CommandDescription *c = &command;
                buses.push_back(c);
            }
        }

        return std::move(make_pair(stops, buses));
    }

    void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
        const auto [stops, buses] = SeparateStopsAndBuses();
        for (const auto& stop : stops){
            if (stop){
                catalogue.AddStop(stop -> id,
                                 std::move(ParseCoordinates(stop -> description)));
            }
        }

        for (const auto& stop : stops){
            if (stop){
                ParseRealDistances(stop -> id, stop -> description, catalogue);
            }
        }
        for (const auto& bus : buses){
            if (bus){
                catalogue.AddBus(bus -> id, ParseRoute(bus -> description));
            }
        }
    }
}