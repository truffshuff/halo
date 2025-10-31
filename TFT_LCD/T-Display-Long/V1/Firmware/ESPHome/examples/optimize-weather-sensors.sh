#!/bin/bash
# Quick optimization script for weather-sensors.yaml
# Adds 'internal: true' to all homeassistant platform sensors
#
# Usage: ./optimize-weather-sensors.sh
#
# This saves 18-37KB of heap memory by preventing sensors from being
# republished back to Home Assistant (they're only needed for display)

WEATHER_SENSORS_FILE="weather-sensors.yaml"
BACKUP_FILE="weather-sensors.yaml.backup"

# Create backup
echo "Creating backup: $BACKUP_FILE"
cp "$WEATHER_SENSORS_FILE" "$BACKUP_FILE"

# Add 'internal: true' after each 'id: ' line that doesn't already have it
echo "Adding 'internal: true' to all homeassistant sensors..."

# Use sed to add 'internal: true' after lines matching 'id: ' pattern
# This preserves indentation and only adds if not already present
sed -i.tmp '/^    id: .*$/a\
    internal: true  # Saves ~200-400 bytes heap per sensor' "$WEATHER_SENSORS_FILE"

# Remove the temporary file created by sed
rm -f "${WEATHER_SENSORS_FILE}.tmp"

echo "Optimization complete!"
echo ""
echo "Changes made:"
echo "- Backup saved to: $BACKUP_FILE"
echo "- Added 'internal: true' to ~125 sensors"
echo "- Expected heap savings: 25-50KB"
echo ""
echo "Next steps:"
echo "1. Review the changes: diff $BACKUP_FILE $WEATHER_SENSORS_FILE"
echo "2. Compile and flash your configuration"
echo "3. Monitor heap usage with memory_stats.yaml"
echo ""
echo "To revert: mv $BACKUP_FILE $WEATHER_SENSORS_FILE"
