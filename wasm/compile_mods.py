from pathlib import Path
from compile_mod import compile

for path in Path('../sources/').glob('*/'):
	compile(path.name)