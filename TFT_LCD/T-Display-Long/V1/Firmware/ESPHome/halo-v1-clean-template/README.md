# halo-v1 Clean Template

## Overview
The halo-v1 Clean Template is designed to provide a streamlined configuration for the halo-v1 project, focusing on essential packages and functionalities. This template includes only the active/default packages and incorporates the on_load functionality to ensure a smooth startup process.

## Project Structure
The project is organized into several key directories and files:

- **halo-v1-template.yaml**: The main configuration file that includes active packages and references the on_load functionality.
- **packages/**: Contains configuration files for active packages, LVGL pages, and additional packages.
  - **active_packages.yaml**: Lists the essential components required for the project.
  - **lvgl_pages.yaml**: Defines the layout and structure of the user interface.
  - **packages-extras.yaml**: Includes additional packages that can be referenced as needed.
- **includes/**: Contains reusable configurations and templates.
  - **on_load.yaml**: Specifies actions or configurations executed when the firmware is loaded.
  - **common_templates.yaml**: Contains common templates for consistency across configurations.
- **docs/examples/**: Provides documentation for users.
  - **quick-start.md**: A concise guide for setting up and running the firmware.
  - **custom-pages.md**: Step-by-step instructions for creating custom LVGL pages.
- **.github/workflows/**: Defines the continuous integration workflow for the project.
  - **ci-esphome.yml**: Specifies steps to run the ESPHome compile command for validation.
- **.gitignore**: Specifies files and directories to be ignored by version control.

## Getting Started
To get started with the halo-v1 Clean Template, follow the instructions in the **docs/examples/quick-start.md** file for a minimal setup. For more advanced configurations, refer to the **docs/examples/custom-pages.md** for guidance on creating custom LVGL pages.

## Contributing
Contributions to the halo-v1 Clean Template are welcome! Please follow the standard practices for submitting issues and pull requests.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.