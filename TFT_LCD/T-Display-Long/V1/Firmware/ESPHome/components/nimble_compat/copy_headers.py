Import("env")
import shutil
import os

# Get the project directory
project_dir = env.get("PROJECT_DIR")
custom_components = os.path.join(project_dir, "custom_components")

# Find ESP-IDF framework include path
framework_dir = env.PioPlatform().get_package_dir("framework-espidf")
bt_include = os.path.join(framework_dir, "components", "bt", "include", "esp32", "include")

print("Custom components path:", custom_components)
print("BT include path:", bt_include)

# Copy our shim headers to ESP-IDF BT include directory
if os.path.exists(custom_components) and os.path.exists(bt_include):
    for header in ["esp_bt_defs.h", "esp_gap_ble_api.h", "esp_gattc_api.h"]:
        src = os.path.join(custom_components, header)
        dst = os.path.join(bt_include, header)
        if os.path.exists(src):
            print(f"Copying {header} to ESP-IDF include path")
            shutil.copy2(src, dst)
        else:
            print(f"Warning: {src} not found")
else:
    print("Warning: custom_components or bt_include path not found")
