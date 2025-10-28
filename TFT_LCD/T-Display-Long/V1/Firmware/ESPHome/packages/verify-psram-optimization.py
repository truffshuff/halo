#!/usr/bin/env python3
"""
Verify PSRAM optimization implementation in weather-sensors.yaml
"""

import sys
from pathlib import Path

def check_file(filepath):
    """Check if file has been properly converted"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Checks
    checks = {
        'No char buffer[] remaining': content.count('char buffer[') == 0,
        'Uses psram_buffers_initialized': 'psram_buffers_initialized' in content,
        'Uses psram_temp_buffer': 'psram_temp_buffer' in content,
        'Uses psram_condition_buffer': 'psram_condition_buffer' in content,
        'Has memory monitoring sensors': 'Free Internal Heap' in content,
        'Has PSRAM optimization header': 'PSRAM OPTIMIZATION ENABLED' in content,
    }
    
    # Count conversions
    psram_usage_count = content.count('id(psram_')
    
    return checks, psram_usage_count

def main():
    filepath = Path('weather-sensors.yaml')
    
    if not filepath.exists():
        print(f"❌ Error: {filepath} not found")
        print("   Run this script from the packages/ directory")
        sys.exit(1)
    
    print("=" * 70)
    print("🔍 PSRAM Optimization Verification")
    print("=" * 70)
    print()
    
    checks, usage_count = check_file(filepath)
    
    print("📋 Verification Checklist:")
    print()
    
    all_passed = True
    for check, passed in checks.items():
        status = "✅" if passed else "❌"
        print(f"  {status} {check}")
        if not passed:
            all_passed = False
    
    print()
    print(f"📊 Statistics:")
    print(f"  • PSRAM buffer references: {usage_count}")
    print(f"  • File size: {filepath.stat().st_size:,} bytes")
    print()
    
    if all_passed:
        print("=" * 70)
        print("✅ SUCCESS! Weather sensors are fully PSRAM optimized!")
        print("=" * 70)
        print()
        print("Next steps:")
        print("  1. Verify psram-helpers.yaml exists")
        print("  2. Update main config to include packages in correct order:")
        print("     packages:")
        print("       psram_helpers: !include packages/psram-helpers.yaml")
        print("       weather_sensors: !include packages/weather-sensors.yaml")
        print("  3. Compile and monitor memory usage")
        print()
        return 0
    else:
        print("=" * 70)
        print("⚠️  Some checks failed. Please review the issues above.")
        print("=" * 70)
        return 1

if __name__ == '__main__':
    sys.exit(main())
