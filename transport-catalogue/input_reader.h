#pragma once
#include <list>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"


namespace transport_directory::input_reader{

    struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // Название команды
        std::string id;           // id маршрута или остановки
        std::string description;  // Параметры команды
    };

    using command_pointers = std::list<const CommandDescription*>;

    class InputReader {
    public:
        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        void ParseLine(std::string_view line);

        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        void ApplyCommands(TransportCatalogue& catalogue) const;

        const std::pair<command_pointers, command_pointers> SeparateStopsAndBuses() const;

    private:
        std::vector<CommandDescription> commands_;
    };

    std::string_view Trim(std::string_view string);
}