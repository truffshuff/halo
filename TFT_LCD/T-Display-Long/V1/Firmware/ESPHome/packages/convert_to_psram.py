#!/usr/bin/env python3
"""
Convert weather-sensors.yaml to use PSRAM buffers instead of stack allocations
"""

import re
import sys

def convert_simple_buffer(match_obj):
    """Convert simple char buffer[] patterns to PSRAM"""
    indent = match_obj.group(1)
    buffer_size = match_obj.group(2)
    snprintf_call = match_obj.group(3)
    return_stmt = match_obj.group(4)
    
    # Replace with PSRAM version
    result = f'''{indent}if (id(psram_buffers_initialized)) {{
{indent}  {snprintf_call.replace("buffer", "id(psram_temp_buffer)").replace(f"sizeof(buffer)", "64")}
{indent}  return id(psram_temp_buffer);
{indent}}}
{indent}return "--";'''
    return result

def convert_complex_buffer_with_high_low(content):
    """Convert complex patterns that combine high and low temps"""
    # Pattern for Day 2-10 high temps with H:X° L:Y° format
    pattern = r'([ ]+)text: !lambda \|-\n\1  char buffer\[32\];\n\1  char high_str\[8\];\n\1  char low_str\[8\];\n\1  const char \*high_text = "--";\n\1  const char \*low_text = "--";\n\1  float low = id\(([^)]+)\)\.state;\n\1  if \(!isnan\(x\)\) \{\n\1    snprintf\(high_str, sizeof\(high_str\), "%.0f", x\);\n\1    high_text = high_str;\n\1  \}\n\1  if \(!isnan\(low\)\) \{\n\1    snprintf\(low_str, sizeof\(low_str\), "%.0f", low\);\n\1    low_text = low_str;\n\1  \}\n\1  snprintf\(buffer, sizeof\(buffer\), "([^"]+)", high_text, low_text\);\n\1  return buffer;'
    
    def replace_func(m):
        indent = m.group(1)
        low_temp_id = m.group(2)
        format_str = m.group(3)
        fallback = format_str.replace("%s", "--")
        
        return f'''{indent}text: !lambda |-
{indent}  if (id(psram_buffers_initialized)) {{
{indent}    const char *high_text = "--";
{indent}    const char *low_text = "--";
{indent}    char high_str[8];
{indent}    char low_str[8];
{indent}    float low = id({low_temp_id}).state;
{indent}    
{indent}    if (!isnan(x)) {{
{indent}      snprintf(high_str, sizeof(high_str), "%.0f", x);
{indent}      high_text = high_str;
{indent}    }}
{indent}    if (!isnan(low)) {{
{indent}      snprintf(low_str, sizeof(low_str), "%.0f", low);
{indent}      low_text = low_str;
{indent}    }}
{indent}    snprintf(id(psram_temp_buffer), 64, "{format_str}", high_text, low_text);
{indent}    return id(psram_temp_buffer);
{indent}  }}
{indent}  return "{fallback}";'''
    
    return re.sub(pattern, replace_func, content)

def convert_low_temp_with_condition(content):
    """Convert low temp patterns that include condition text"""
    # Pattern for low temps with 16-byte buffer (lines around 364)
    pattern = r'([ ]+)text: !lambda \|-\n\1  char buffer\[16\];\n\1  if \(isnan\(x\)\) \{\n\1    snprintf\(buffer, sizeof\(buffer\), "([^"]+)"\);\n\1  \} else \{\n\1    snprintf\(buffer, sizeof\(buffer\), "([^"]+)", x\);\n\1  \}\n\1  return buffer;'
    
    def replace_func(m):
        indent = m.group(1)
        nan_format = m.group(2)
        normal_format = m.group(3)
        
        return f'''{indent}text: !lambda |-
{indent}  if (id(psram_buffers_initialized)) {{
{indent}    if (isnan(x)) {{
{indent}      snprintf(id(psram_temp_buffer), 64, "{nan_format}");
{indent}    }} else {{
{indent}      snprintf(id(psram_temp_buffer), 64, "{normal_format}", x);
{indent}    }}
{indent}    return id(psram_temp_buffer);
{indent}  }}
{indent}  return "{nan_format}";'''
    
    return re.sub(pattern, replace_func, content)

def convert_simple_snprintf(content):
    """Convert simple single snprintf patterns"""
    # Pattern for simple snprintf with return
    pattern = r'([ ]+)text: !lambda \|-\n\1  char buffer\[\d+\];\n\1  snprintf\(buffer, sizeof\(buffer\), "([^"]+)"([^)]*)\);\n\1  return buffer;'
    
    def replace_func(m):
        indent = m.group(1)
        format_str = m.group(2)
        args = m.group(3)
        
        return f'''{indent}text: !lambda |-
{indent}  if (id(psram_buffers_initialized)) {{
{indent}    snprintf(id(psram_temp_buffer), 64, "{format_str}"{args});
{indent}    return id(psram_temp_buffer);
{indent}  }}
{indent}  return "--";'''
    
    return re.sub(pattern, replace_func, content)

def convert_condition_text(content):
    """Convert condition text with string concatenation"""
    # Pattern for condition sensors (buffer[30])
    pattern = r'([ ]+)text: !lambda \|-\n\1  char buffer\[30\];\n\1  snprintf\(buffer, sizeof\(buffer\), "([^"]+)", ([^)]+)\);\n\1  return buffer;'
    
    def replace_func(m):
        indent = m.group(1)
        format_str = m.group(2)
        arg = m.group(3)
        
        return f'''{indent}text: !lambda |-
{indent}  if (id(psram_buffers_initialized)) {{
{indent}    snprintf(id(psram_condition_buffer), 128, "{format_str}", {arg});
{indent}    return id(psram_condition_buffer);
{indent}  }}
{indent}  return "--";'''
    
    return re.sub(pattern, replace_func, content)

def main():
    filename = 'weather-sensors.yaml'
    
    print(f"Reading {filename}...")
    with open(filename, 'r') as f:
        content = f.read()
    
    original_count = content.count('char buffer[')
    print(f"Found {original_count} char buffer[] declarations")
    
    # Apply conversions in order
    print("Converting complex high/low temp patterns...")
    content = convert_complex_buffer_with_high_low(content)
    
    print("Converting low temp with condition patterns...")
    content = convert_low_temp_with_condition(content)
    
    print("Converting condition text patterns...")
    content = convert_condition_text(content)
    
    print("Converting simple snprintf patterns...")
    content = convert_simple_snprintf(content)
    
    # Check results
    remaining_count = content.count('char buffer[')
    print(f"Remaining char buffer[] declarations: {remaining_count}")
    print(f"Converted: {original_count - remaining_count}")
    
    # Save backup
    backup_filename = filename + '.backup'
    print(f"Saving backup to {backup_filename}...")
    with open(backup_filename, 'w') as f:
        with open(filename, 'r') as orig:
            f.write(orig.read())
    
    # Write converted file
    print(f"Writing converted file to {filename}...")
    with open(filename, 'w') as f:
        f.write(content)
    
    print("✓ Conversion complete!")
    print("\nNext steps:")
    print("1. Review the changes: diff weather-sensors.yaml.backup weather-sensors.yaml")
    print("2. Ensure psram-helpers.yaml is included BEFORE weather-sensors.yaml")
    print("3. Compile and monitor memory usage")

if __name__ == '__main__':
    main()
