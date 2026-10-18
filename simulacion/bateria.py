#!/usr/bin/env python3
"""Entrada compatible de la batería; implementación en el paquete Python."""
from pathlib import Path
import sys
if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from simulacion.python.bateria import main
if __name__ == "__main__":
    main()
