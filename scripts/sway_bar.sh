
#!/bin/bash

while true; do
    # Date and time
    date_and_week=$(date "+%Y/%m/%d")
    current_time=$(date "+%H:%M")

    #############
    # Battery
    #############
    battery_info=""

    if command -v acpi &>/dev/null; then
        battery_lines=$(acpi -b)
        battery_percentages=($(echo "$battery_lines" | grep -o '[0-9]\+%' | tr -d '%'))

        count=${#battery_percentages[@]}
        if [ $count -eq 1 ]; then
            battery_info="${battery_percentages[0]}%"
        elif [ $count -eq 2 ]; then
            battery_info=" ${battery_percentages[0]}%  ${battery_percentages[1]}%"
        else
            battery_info="?"
        fi
    else
        battery_info="?"
    fi
    #############
    # Audio
    #############

    audio_volume=$(amixer get Master | grep -o '[0-9]*%' | head -1 | tr -d '%')

    #############
    # Brightness
    #############

    brightness=""
    brightness_icon=" "
    if command -v brightnessctl &>/dev/null; then
        brightness=$(brightnessctl get)
        max_brightness=$(brightnessctl max)
        if [ "$max_brightness" -gt 0 ]; then
            brightness_percent=$((brightness * 100 / max_brightness))
        else
            brightness_percent="?"
        fi
    else
        brightness_percent="?"
    fi

    #############
    # Headset Battery
    #############

    headset_battery=""
    headset_icon="🎧"
    headset_path=$(upower --enumerate | grep 'headset' | head -n 1)

    if [ -n "$headset_path" ]; then
        headset_percentage=$(upower -i "$headset_path" | grep -E "percentage" | awk '{print $2}' | tr -d '%')
        if [ -n "$headset_percentage" ]; then
            headset_battery="$headset_icon ${headset_percentage}%"
        fi
    fi

    #############
    # Final Output
    #############

    echo "|  ${audio_volume:-?}% | $brightness_icon${brightness_percent:-?}% | ${battery_info:-?} | ${headset_battery:-} | $date_and_week  $current_time"

    sleep 0.5
done

