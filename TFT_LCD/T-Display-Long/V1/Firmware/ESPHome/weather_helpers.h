// Weather Helper Functions for HALO v1 ESPHome Configuration
// Copied from https://github.com/truffshuff/esphome-components/components/weather_helpers/weather_helpers.h

#pragma once

#include <string>
#include <algorithm>
#include <cmath>

namespace weather_helpers {

inline int convert_to_local_hour(const std::string& datetime_str, int32_t local_tz_offset) {
    if (datetime_str.length() < 13) return -1;
    std::string hour_str = datetime_str.substr(11, 2);
    int hour = std::stoi(hour_str);
    int source_offset_hours = 0;
    if (datetime_str.length() >= 20) {
        if (datetime_str[19] == 'Z') source_offset_hours = 0;
        else if (datetime_str.length() >= 24 && (datetime_str[19] == '+' || datetime_str[19] == '-')) {
            source_offset_hours = std::stoi(datetime_str.substr(20, 2));
            if (datetime_str[19] == '-') source_offset_hours = -source_offset_hours;
        }
    }
    int local_offset_hours = local_tz_offset / 3600;
    int utc_hour = hour - source_offset_hours;
    hour = utc_hour + local_offset_hours;
    while (hour < 0) hour += 24;
    while (hour >= 24) hour -= 24;
    return hour;
}

inline std::string format_hour(int hour) {
    if (hour < 0 || hour > 23) return "--:--";
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:00", hour);
    return std::string(buf);
}

inline bool parse_forecast_date(const std::string& datetime_str, int32_t local_tz_offset, int& month, int& day) {
    if (datetime_str.length() < 19) return false;
    int year = std::stoi(datetime_str.substr(0, 4));
    month = std::stoi(datetime_str.substr(5, 2));
    day = std::stoi(datetime_str.substr(8, 2));
    int hour = std::stoi(datetime_str.substr(11, 2));
    if (datetime_str.length() >= 25 && (datetime_str[19] == '+' || datetime_str[19] == '-')) {
        int offset_hours = std::stoi(datetime_str.substr(20, 2));
        if (datetime_str[19] == '-') offset_hours = -offset_hours;
        int local_offset_hours = local_tz_offset / 3600;
        hour += (local_offset_hours - offset_hours);
        if (hour < 0) {
            day--;
            if (day < 1) {
                month--;
                if (month < 1) { month = 12; year--; }
                int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
                if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) day = 29;
                else day = days_in_month[month-1];
            }
        } else if (hour >= 24) {
            day++;
            int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
            int max_days = days_in_month[month-1];
            if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) max_days = 29;
            if (day > max_days) { day = 1; month++; if (month > 12) { month = 1; year++; } }
        }
    }
    return true;
}

inline std::string get_weather_icon(const std::string& condition) {
    if (condition.empty()) return "\U000F14E4";
    if (condition.find("lightning-rainy") != std::string::npos) return "\U000F067E";
    if (condition.find("snowy-rainy") != std::string::npos) return "\U000F067F";
    if (condition.find("clear-night") != std::string::npos) return "\U000F0594";
    if (condition.find("partlycloudy") != std::string::npos) return "\U000F0595";
    if (condition.find("lightning") != std::string::npos) return "\U000F0593";
    if (condition.find("snowy") != std::string::npos) return "\U000F0598";
    if (condition.find("rainy") != std::string::npos) return "\U000F0597";
    if (condition.find("pouring") != std::string::npos) return "\U000F0596";
    if (condition.find("hail") != std::string::npos) return "\U000F0592";
    if (condition.find("fog") != std::string::npos) return "\U000F0591";
    if (condition.find("windy-variant") != std::string::npos) return "\U000F059E";
    if (condition.find("windy") != std::string::npos) return "\U000F059D";
    if (condition.find("cloudy") != std::string::npos) return "\U000F0590";
    if (condition.find("sunny") != std::string::npos || condition.find("clear") != std::string::npos) return "\U000F0599";
    if (condition.find("exceptional") != std::string::npos) return "\U000F0F2F";
    return "\U000F0599";
}

inline uint32_t get_weather_icon_color(const std::string& condition) {
    if (condition.find("clear-night") != std::string::npos) return 0x6A5ACD;
    if (condition.find("sunny") != std::string::npos || condition.find("clear") != std::string::npos) return 0xFFD700;
    if (condition.find("snowy-rainy") != std::string::npos) return 0xAFEEEE;
    if (condition.find("lightning-rainy") != std::string::npos || condition.find("lightning") != std::string::npos) return 0xFF1493;
    if (condition.find("partlycloudy") != std::string::npos) return 0x87CEFA;
    if (condition.find("cloudy") != std::string::npos) return 0xA9A9A9;
    if (condition.find("rainy") != std::string::npos || condition.find("pouring") != std::string::npos) return 0x4169E1;
    if (condition.find("snowy") != std::string::npos) return 0xDCDCDC;
    if (condition.find("fog") != std::string::npos) return 0xB0C4DE;
    if (condition.find("windy") != std::string::npos) return 0x90EE90;
    return 0xFFFFFF;
}

inline std::string format_condition_text(const std::string& condition) {
    if (condition.empty()) return "Unknown";
    if (condition == "clear-night") return "Clear Night";
    if (condition == "cloudy") return "Cloudy";
    if (condition == "exceptional") return "Exceptional";
    if (condition == "fog") return "Fog";
    if (condition == "hail") return "Hail";
    if (condition == "lightning") return "Lightning";
    if (condition == "lightning-rainy") return "Lightning & Rain";
    if (condition == "partlycloudy") return "Partly Cloudy";
    if (condition == "pouring") return "Pouring";
    if (condition == "rainy") return "Rainy";
    if (condition == "snowy") return "Snowy";
    if (condition == "snowy-rainy") return "Sleet";
    if (condition == "sunny") return "Sunny";
    if (condition == "windy") return "Windy";
    if (condition == "windy-variant") return "Very Windy";
    std::string result = condition;
    if (!result.empty()) result[0] = toupper(result[0]);
    return result;
}

inline std::string bearing_to_direction(float bearing) {
    if (std::isnan(bearing)) return "--";
    if (bearing >= 337.5 || bearing < 22.5) return "N";
    if (bearing >= 22.5 && bearing < 67.5) return "NE";
    if (bearing >= 67.5 && bearing < 112.5) return "E";
    if (bearing >= 112.5 && bearing < 157.5) return "SE";
    if (bearing >= 157.5 && bearing < 202.5) return "S";
    if (bearing >= 202.5 && bearing < 247.5) return "SW";
    if (bearing >= 247.5 && bearing < 292.5) return "W";
    return "NW";
}

inline std::string format_temp(float temp, const char* default_str = "--°") {
    if (std::isnan(temp)) return std::string(default_str);
    char buf[16]; snprintf(buf, sizeof(buf), "%.0f°", temp); return std::string(buf);
}

inline std::string format_temp_precise(float temp, const char* default_str = "--.-°") {
    if (std::isnan(temp)) return std::string(default_str);
    char buf[16]; snprintf(buf, sizeof(buf), "%.1f°", temp); return std::string(buf);
}

inline std::string format_percent(float value, const char* default_str = "--%") {
    if (std::isnan(value)) return std::string(default_str);
    char buf[16]; snprintf(buf, sizeof(buf), "%.0f%%", value); return std::string(buf);
}

inline std::string format_precipitation(float value, const char* default_str = "--\"") {
    if (std::isnan(value)) return std::string(default_str);
    char buf[16]; snprintf(buf, sizeof(buf), "%.2f\"", value); return std::string(buf);
}

inline std::string format_numeric(float value, const char* default_str = "--") {
    if (std::isnan(value)) return std::string(default_str);
    char buf[16]; snprintf(buf, sizeof(buf), "%.0f", value); return std::string(buf);
}

inline std::string format_hi_lo_temps(float high, float low) {
    if (std::isnan(high) || std::isnan(low)) return "H:--° L:--°";
    char buf[64]; snprintf(buf, sizeof(buf), "#FF0000 H:%.0f°# #00DFFF L:%.0f°#", high, low); return std::string(buf);
}

} // namespace weather_helpers
