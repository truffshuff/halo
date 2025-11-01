# Quick Start Guide for Halo V1 Project

Welcome to the Halo V1 Project! This quick start guide will help you set up and run the firmware with minimal steps and memory-safe defaults.

## Prerequisites

- Ensure you have ESPHome installed and configured on your system.
- Familiarity with YAML configuration files.

## Quick Setup Steps

1. **Clone the Repository**
   ```bash
   git clone https://github.com/yourusername/halo-v1-clean-template.git
   cd halo-v1-clean-template
   ```

2. **Configure Your Device**
   - Open `halo-v1-template.yaml` in your preferred text editor.
   - Modify the `friendly_name` and `device_name` fields to suit your device.

3. **Select Your Configuration**
   - Choose one of the predefined configurations (e.g., Config 1 or Config 7) based on your needs.
   - Ensure that the `buffer_size` is set appropriately:
     - For BLE: `buffer_size: 15%`
     - For Full Weather (no BLE): `buffer_size: 25%`
     - For Clock-only: `buffer_size: 35%`

4. **Compile the Firmware**
   - Run the following command to compile the firmware:
   ```bash
   esphome halo-v1-template.yaml run
   ```

5. **Upload to Your Device**
   - Connect your device via USB and upload the firmware:
   ```bash
   esphome halo-v1-template.yaml upload
   ```

6. **Monitor the Logs**
   - After uploading, monitor the logs to ensure everything is functioning correctly:
   ```bash
   esphome halo-v1-template.yaml logs
   ```

## Memory-Safe Defaults

The default configurations are designed to be memory-safe. If you encounter any issues, consider the following:

- Check the enabled packages in `active_packages.yaml`.
- Ensure that you are not exceeding the memory limits based on your selected configuration.

## Additional Resources

For more detailed instructions on creating custom LVGL pages, refer to the [Custom Pages Documentation](custom-pages.md).

Happy coding!