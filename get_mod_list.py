import json
import os
from pathlib import Path
import re


def get_mod_name(item: str) -> str:
    """Get the mod subdirectory name for the given path."""
    mod_dir = Path("sources")
    valid_mod_names = [item.name for item in mod_dir.iterdir() if item.is_dir()]

    item_path = Path(item)

    if item_path.name in valid_mod_names:
        return item_path.name
    else:
        if not item_path.is_relative_to("sources"):
            return ""
        else:
            mod_name = item_path.parents[-3].stem
            if mod_name in valid_mod_names:
                return mod_name


if __name__ == "__main__":
    # Get the list of changed files if
    mods_to_build = [get_mod_name(item) for item in re.split(r'[;:,\s]+', os.environ.get("MODS", ""))]
    if not mods_to_build or mods_to_build == [""]:
        mod_dir = Path("sources")
        mods_to_build  = [item.name for item in mod_dir.iterdir() if item.is_dir()]
    print(json.dumps(list(set(mods_to_build))))
