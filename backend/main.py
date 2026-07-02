# =============================================================================
# backend/main.py — API FastAPI para el Compilador CS#
# =============================================================================
# Endpoints:
#   GET  /health          → estado del servidor
#   POST /compile         → compilar código fuente → devuelve .s + AST
#   POST /run             → ensamblar + ejecutar → devuelve stdout
#
# Cómo correr (en WSL):
#   pip install fastapi uvicorn
#   COMPILER_PATH=../416608878/compilador uvicorn main:app --reload --port 8000
# =============================================================================

import json
import os
import subprocess
import tempfile
from pathlib import Path

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel


COMPILER_PATH = os.environ.get(
    "COMPILER_PATH",
    str(Path(__file__).parent.parent / "compiler" / "compilador")
)

TIMEOUT_SEC = 15  # segundos máximos para compilación/ejecución


app = FastAPI(title="CS# Compiler API", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


class CompileRequest(BaseModel):
    code: str


class RunRequest(BaseModel):
    assembly: str


# -----------------------------------------------------------------------------
# Endpoints
# -----------------------------------------------------------------------------

@app.get("/health")
def health():
    compiler_exists = Path(COMPILER_PATH).exists()
    return {
        "status": "ok",
        "compiler": COMPILER_PATH,
        "compiler_found": compiler_exists,
    }


@app.post("/compile")
def compile_code(req: CompileRequest):
    """
    Recibe código fuente C#, lo compila con el compilador del proyecto.
    Devuelve:
      - success: bool
      - assembly: código ensamblador x86-64 generado
      - ast: AST en formato JSON (si el modo --ast funciona)
      - messages: salida del compilador (stdout)
      - error: mensaje de error (si success=False)
    """
    # Escribir código fuente a archivo temporal
    with tempfile.NamedTemporaryFile(
        suffix=".cs", mode="w", delete=False, encoding="utf-8"
    ) as f:
        f.write(req.code)
        src_path = f.name

    asm_path = src_path.replace(".cs", ".s")

    try:
        # --- Paso 1: Compilar ---
        result = subprocess.run(
            [COMPILER_PATH, src_path],
            capture_output=True,
            text=True,
            timeout=TIMEOUT_SEC,
        )

        messages = result.stdout + result.stderr

        if result.returncode != 0:
            return {
                "success": False,
                "error": messages or "Error desconocido en la compilación.",
                "assembly": "",
                "ast": None,
                "messages": messages,
            }

        # Leer ensamblador generado
        assembly = ""
        if Path(asm_path).exists():
            assembly = Path(asm_path).read_text(encoding="utf-8")

        # --- Paso 2: Obtener AST ---
        ast_data = None
        try:
            ast_result = subprocess.run(
                [COMPILER_PATH, src_path, "--ast"],
                capture_output=True,
                text=True,
                timeout=TIMEOUT_SEC,
            )
            if ast_result.returncode == 0 and ast_result.stdout.strip():
                stdout = ast_result.stdout.strip()
                json_start = stdout.find('{')
                if json_start != -1:
                    ast_data = json.loads(stdout[json_start:])
        except (subprocess.TimeoutExpired, json.JSONDecodeError, Exception):
            ast_data = None

        return {
            "success": True,
            "assembly": assembly,
            "ast": ast_data,
            "messages": messages,
            "error": "",
        }

    except subprocess.TimeoutExpired:
        return {
            "success": False,
            "error": "Timeout: la compilación tardó demasiado.",
            "assembly": "",
            "ast": None,
            "messages": "",
        }
    except FileNotFoundError:
        return {
            "success": False,
            "error": f"Compilador no encontrado en: {COMPILER_PATH}\n"
                     "Asegúrate de compilar el proyecto con g++ primero y\n"
                     "de configurar la variable COMPILER_PATH.",
            "assembly": "",
            "ast": None,
            "messages": "",
        }
    finally:
        for p in [src_path, asm_path]:
            try:
                Path(p).unlink(missing_ok=True)
            except Exception:
                pass


@app.post("/run")
def run_code(req: RunRequest):
    """
    Recibe código ensamblador x86-64, lo enlaza con gcc y lo ejecuta.
    Devuelve:
      - success: bool
      - output: stdout + stderr del programa
    """
    with tempfile.NamedTemporaryFile(
        suffix=".s", mode="w", delete=False, encoding="utf-8"
    ) as f:
        f.write(req.assembly)
        asm_path = f.name

    exe_path = asm_path.replace(".s", "")

    try:
        # --- Enlazar ---
        link = subprocess.run(
            ["gcc", "-no-pie", asm_path, "-o", exe_path],
            capture_output=True,
            text=True,
            timeout=TIMEOUT_SEC,
        )

        if link.returncode != 0:
            return {
                "success": False,
                "output": "Error al enlazar con gcc:\n" + link.stderr,
            }

        # --- Ejecutar ---
        run = subprocess.run(
            [exe_path],
            capture_output=True,
            text=True,
            timeout=TIMEOUT_SEC,
        )

        output = run.stdout
        if run.returncode != 0:
            output += f"\n[Exit code: {run.returncode}]"
        if run.stderr:
            output += "\n[stderr]\n" + run.stderr

        return {"success": run.returncode == 0, "output": output}

    except subprocess.TimeoutExpired:
        return {
            "success": False,
            "output": "Timeout: el programa tardó demasiado en ejecutarse.",
        }
    except FileNotFoundError:
        return {
            "success": False,
            "output": "Error: gcc no está disponible. Instálalo con:\n  sudo apt install gcc",
        }
    finally:
        for p in [asm_path, exe_path]:
            try:
                Path(p).unlink(missing_ok=True)
            except Exception:
                pass
