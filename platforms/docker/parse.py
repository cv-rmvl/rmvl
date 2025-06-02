import os
import yaml

with open(os.path.join(os.path.dirname(__file__), 'images.yml'), 'r') as file:
    datas: list[dict] = yaml.safe_load(file)

for data in datas:
    name = data.get('name')
    if not name:
        raise ValueError("Each image data must have a 'name' field.")
    image = data.get('image')
    if not image:
        raise ValueError(f"Image '{name}' must have an 'image' field.")
    options = data.get('options', [])
    if isinstance(options, list):
        options = ' '.join(options)
    cmd = data.get('cmd', '/bin/bash')

    print(f"{name}#{image}#{options}#{cmd}")
