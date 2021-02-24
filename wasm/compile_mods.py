from pathlib import Path
from compile_mod import compile
import sys

debug = (sys.argv[1].lower() == 'debug') if len(sys.argv) > 1 else True;

for path in Path('../sources/').glob('*/'):
	compile(path.name, debug)