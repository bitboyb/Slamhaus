import os
import re

def find_css_files(root):
    for dirpath, _, filenames in os.walk(root):
        for file in filenames:
            if file.endswith(".css"):
                yield os.path.join(dirpath, file)

def extract_variables(palette_text):
    return re.findall(r"--(color-\d+):\s*[^;]+;", palette_text)

def build_usage_map(vars_list, css_files):
    usage_map = {var: 0 for var in vars_list}
    for file in css_files:
        with open(file, 'r', encoding='utf-8') as f:
            content = f.read()
            for var in vars_list:
                # Count occurrences beyond the definition
                usage_map[var] += len(re.findall(rf"\b{var}\b", content)) - (
                    1 if file.endswith("palette.css") else 0
                )
    return usage_map

def remap_variables(vars_list, usage_map):
    used_vars = [v for v in vars_list if usage_map[v] > 0]
    new_names = {old: f"color-{i:02}" for i, old in enumerate(used_vars)}
    return new_names

def update_palette(palette_text, var_rename_map):
    lines = palette_text.splitlines()
    new_lines = []
    for line in lines:
        match = re.match(r"\s*--(color-\d+):\s*[^;]+;", line)
        if match:
            var = match.group(1)
            if var in var_rename_map:
                new_var = var_rename_map[var]
                new_line = re.sub(var, new_var, line)
                new_lines.append(new_line)
            else:
                continue  # Remove unused variable
        else:
            new_lines.append(line)
    return "\n".join(new_lines)

def update_references(css_files, var_rename_map):
    for file in css_files:
        with open(file, 'r', encoding='utf-8') as f:
            content = f.read()
        for old, new in var_rename_map.items():
            content = re.sub(rf"\b{old}\b", new, content)
        with open(file, 'w', encoding='utf-8') as f:
            f.write(content)

def main():
    root = os.path.dirname(os.path.abspath(__file__))
    palette_path = os.path.join(root, "palette.css")

    with open(palette_path, 'r', encoding='utf-8') as f:
        palette_text = f.read()

    vars_list = extract_variables(palette_text)
    css_files = list(find_css_files(root))
    usage_map = build_usage_map(vars_list, css_files)
    rename_map = remap_variables(vars_list, usage_map)

    updated_palette = update_palette(palette_text, rename_map)
    with open(palette_path, 'w', encoding='utf-8') as f:
        f.write(updated_palette)

    update_references(css_files, rename_map)
    print("✅ Palette cleaned and variables remapped.")

if __name__ == "__main__":
    main()