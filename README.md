# CS# Compiler IDE — Interfaz Gráfica

Aplicación web que demuestra las fases del compilador de forma interactiva:
editor de código → ensamblador x86-64 → visualización del AST → ejecución.

---

## Estructura

```
PROYECTOFINAL/
  compiler/   -> Código en C para el compilador
  backend/    → FastAPI (corre en WSL/Linux)
  frontend/   → React + Vite (corre en Windows o WSL)
```

---

## Paso 1 — Compilar el compilador C++

En WSL, desde la carpeta `compiler/`:

```bash

g++ -std=c++17 -O2 main.cpp scanner.cpp token.cpp parser.cpp ast.cpp visitor.cpp ast_printer.cpp -o compilador

```

Verifica que funciona:

```bash
echo 'int main() { return 0; }' > test.cs
./compilador test.cs          # genera test.s
./compilador test.cs --ast    # imprime JSON del AST
```

---

# Paso 2- Backend

Desde WSL, en la carpeta `backend/`:

```bash
cd /mnt/d/Desktop/UTEC/COMPILADORES/PROYECTOFINAL/backend

# Crear y activar el virtual environment
python3 -m venv venv
source venv/bin/activate

# Instalar dependencias
pip install -r requirements.txt

# Levantar el servidor (Esta ruta es donde se cojera el compilado, es importante ya que sino no se podria)
COMPILER_PATH=/mnt/d/Desktop/UTEC/COMPILADORES/PROYECTOFINAL/compiler/compilador \ 
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

La próxima vez que se abrá WSL solo necesitas:

```bash
cd /mnt/d/Desktop/UTEC/COMPILADORES/PROYECTOFINAL/backend
source venv/bin/activate
COMPILER_PATH=/mnt/d/Desktop/UTEC/COMPILADORES/PROYECTOFINAL/compiler/compilador uvicorn main:app --reload --port 8000
```


## Paso 3 — Frontend React (Windows o WSL)

```bash
cd interface/frontend

npm install

npm run dev
```

Abre en el navegador: http://localhost:3000

---

## Uso de la interfaz

| Botón       | Acción                                              |
|-------------|-----------------------------------------------------|
| **Compilar** | Envía el código al backend → genera ensamblador + AST |
| **Ejecutar** | Ensambla con `gcc` y ejecuta el programa            |
| **Reset**    | Vuelve al código de ejemplo                         |

### Tabs del panel derecho

- **⚙️ Ensamblador x86** — código AT&T generado, con coloreado por tipo de instrucción
- **🌳 AST** — árbol de sintaxis abstracta interactivo (expandir/colapsar nodos)
- **▶️ Salida** — stdout del programa ejecutado
- **📋 Mensajes** — mensajes del compilador y errores

---

## Solución de problemas

**"Compilador no encontrado"**
→ Ajusta `COMPILER_PATH` al path absoluto del binario dentro de WSL.

**CORS / conexión rechazada**
→ Verifica que el backend está corriendo en el puerto 8000 antes de abrir el frontend.

**`gcc` no disponible para ejecutar**
→ `sudo apt install gcc` en WSL.
