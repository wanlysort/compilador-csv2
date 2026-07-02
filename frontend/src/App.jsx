import { useState, useCallback } from 'react'
import Editor from '@monaco-editor/react'
import axios from 'axios'

// =============================================================================
// Configuración
// =============================================================================
const API = '/api'   // Proxy de Vite → http://localhost:8000

// =============================================================================
// Código de ejemplo inicial
// =============================================================================
const EXAMPLE_CODE = `// Ejemplo: Fibonacci con structs y funciones
struct Punto {
    int x;
    int y;
}

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    int i = 0;
    while (i <= 10) {
        printf("fib", fibonacci(i));
        i = i + 1;
    }
    return 0;
}
`

// =============================================================================
// Colores por tipo de nodo AST
// =============================================================================
const NODE_COLORS = {
  Program:          '#569cd6',
  Functions:        '#4ec9b0',
  FunDec:           '#4ec9b0',
  Structs:          '#c586c0',
  StructDec:        '#c586c0',
  Globals:          '#9cdcfe',
  Body:             '#6a9955',
  VarDec:           '#9cdcfe',
  VarNode:          '#9cdcfe',
  AssignStm:        '#ce9178',
  ReturnStm:        '#f44747',
  PrintfStm:        '#dcdcaa',
  IfStm:            '#c586c0',
  IfBranch:         '#c586c0',
  ElseIfBranch:     '#c586c0',
  ElseBranch:       '#808080',
  WhileStm:         '#569cd6',
  DoWhileStm:       '#569cd6',
  ForStm:           '#569cd6',
  BinaryExp:        '#d7ba7d',
  UnaryExp:         '#d7ba7d',
  CallExp:          '#dcdcaa',
  NumberExp:        '#b5cea8',
  BoolExp:          '#569cd6',
  StringLiteralExp: '#ce9178',
  LValue:           '#9cdcfe',
  LambdaDecExp:     '#c586c0',
  TypeNode:         '#4ec9b0',
  Condition:        '#d7ba7d',
  InitValue:        '#b5cea8',
  InitList:         '#b5cea8',
  InitExpr:         '#b5cea8',
  default:          '#d4d4d4',
}

function getNodeColor(type) {
  return NODE_COLORS[type] || NODE_COLORS.default
}

// =============================================================================
// Componente: Nodo del árbol AST
// =============================================================================
function ASTNode({ node, depth = 0 }) {
  const [expanded, setExpanded] = useState(depth < 2)

  if (!node || typeof node !== 'object') return null

  const hasChildren = node.children && node.children.length > 0
  const color = getNodeColor(node.type)

  // Construir etiqueta del nodo
  let label = node.type || 'Node'
  if (node.name)       label += `: ${node.name}`
  if (node.value !== undefined) label += ` = ${node.value}`
  if (node.op)         label += ` [${node.op}]`
  if (node.format)     label += ` "${node.format}"`
  if (node.base)       label += `: ${node.base}${node.pointers > 0 ? '*'.repeat(node.pointers) : ''}`

  return (
    <div style={{ marginLeft: depth === 0 ? 0 : 16, marginTop: 2 }}>
      <div
        className="ast-node"
        onClick={() => hasChildren && setExpanded(e => !e)}
        style={{ cursor: hasChildren ? 'pointer' : 'default', color }}
      >
        {hasChildren && (
          <span className="ast-toggle">{expanded ? '▾' : '▸'}</span>
        )}
        {!hasChildren && <span className="ast-toggle ast-leaf">◆</span>}
        <span className="ast-label">{label}</span>

        {/* Metadatos adicionales */}
        {node.returnType && (
          <span className="ast-meta">
            → {node.returnType.base}{node.returnType.pointers > 0 ? '*'.repeat(node.returnType.pointers) : ''}
          </span>
        )}
        {node.params && node.params.length > 0 && (
          <span className="ast-meta">
            ({node.params.map(p => `${p.paramType?.base} ${p.name}`).join(', ')})
          </span>
        )}
        {node.vars && node.vars.length > 0 && (
          <span className="ast-meta">
            [{node.vars.map(v => v.name).join(', ')}]
          </span>
        )}
        {node.members && node.members.length > 0 && (
          <span className="ast-meta">
            {'{'}
            {node.members.map(m => `${m.memberType?.base} ${m.name}`).join('; ')}
            {'}'}
          </span>
        )}
        {node.path && node.path.length > 0 && (
          <span className="ast-meta">
            {'*'.repeat(node.deref || 0)}
            {node.path.map(p => p.id + (p.indices ? `[${p.indices.length}]` : '')).join('.')}
          </span>
        )}
      </div>

      {hasChildren && expanded && (
        <div className="ast-children">
          {node.children.map((child, i) => (
            <ASTNode key={i} node={child} depth={depth + 1} />
          ))}
        </div>
      )}
    </div>
  )
}

// =============================================================================
// Componente: Visualizador del AST
// =============================================================================
function ASTViewer({ ast }) {
  if (!ast) {
    return (
      <div className="empty-state">
        <div className="empty-icon">🌳</div>
        <p>Compila tu código para ver el AST</p>
      </div>
    )
  }

  return (
    <div className="ast-viewer">
      <ASTNode node={ast} depth={0} />
    </div>
  )
}

// =============================================================================
// Componente: Panel de ensamblador
// =============================================================================
function AssemblyPanel({ assembly }) {
  if (!assembly) {
    return (
      <div className="empty-state">
        <div className="empty-icon">⚙️</div>
        <p>Compila tu código para ver el ensamblador x86-64</p>
      </div>
    )
  }

  // Colorear líneas del ensamblador
  const lines = assembly.split('\n')
  return (
    <div className="assembly-viewer">
      <table className="asm-table">
        <tbody>
          {lines.map((line, i) => {
            let cls = 'asm-line'
            const trimmed = line.trim()
            if (trimmed.startsWith('.'))           cls += ' asm-directive'
            else if (trimmed.startsWith('#'))      cls += ' asm-comment'
            else if (trimmed.endsWith(':'))        cls += ' asm-label'
            else if (trimmed.startsWith('//'))     cls += ' asm-comment'
            else if (trimmed.length > 0)           cls += ' asm-instr'
            return (
              <tr key={i} className={cls}>
                <td className="asm-lineno">{i + 1}</td>
                <td className="asm-code">{line}</td>
              </tr>
            )
          })}
        </tbody>
      </table>
    </div>
  )
}

// =============================================================================
// Componente: Panel de salida
// =============================================================================
function OutputPanel({ output, success }) {
  if (output === null || output === undefined) {
    return (
      <div className="empty-state">
        <div className="empty-icon">▶️</div>
        <p>Compila y luego ejecuta para ver la salida</p>
      </div>
    )
  }

  return (
    <div className={`output-viewer ${success ? 'output-ok' : 'output-err'}`}>
      <div className="output-badge">
        {success ? '✅ Ejecución exitosa' : '❌ Error en ejecución'}
      </div>
      <pre className="output-text">{output || '(sin salida)'}</pre>
    </div>
  )
}

// =============================================================================
// App principal
// =============================================================================
export default function App() {
  const [code,     setCode]     = useState(EXAMPLE_CODE)
  const [assembly, setAssembly] = useState('')
  const [ast,      setAst]      = useState(null)
  const [output,   setOutput]   = useState(null)
  const [runOk,    setRunOk]    = useState(null)
  const [messages, setMessages] = useState('')
  const [activeTab, setActiveTab] = useState('assembly')
  const [loading,  setLoading]  = useState(false)
  const [status,   setStatus]   = useState({ type: 'idle', text: 'Listo' })

  // --- Compilar ---
  const handleCompile = useCallback(async () => {
    if (loading) return
    setLoading(true)
    setStatus({ type: 'loading', text: 'Compilando...' })
    setOutput(null)
    setRunOk(null)

    try {
      const res = await axios.post(`${API}/compile`, { code })
      const data = res.data

      setMessages(data.messages || '')

      if (data.success) {
        setAssembly(data.assembly || '')
        setAst(data.ast || null)
        setStatus({ type: 'ok', text: 'Compilación exitosa' })
        setActiveTab('assembly')
      } else {
        setAssembly('')
        setAst(null)
        setMessages(data.error || data.messages || 'Error desconocido')
        setStatus({ type: 'error', text: 'Error en compilación' })
        setActiveTab('messages')
      }
    } catch (err) {
      const msg = err.response?.data?.detail || err.message || 'No se pudo conectar al backend'
      setMessages(msg)
      setStatus({ type: 'error', text: 'Error de conexión' })
    } finally {
      setLoading(false)
    }
  }, [code, loading])

  // --- Ejecutar ---
  const handleRun = useCallback(async () => {
    if (!assembly || loading) return
    setLoading(true)
    setStatus({ type: 'loading', text: 'Ejecutando...' })

    try {
      const res = await axios.post(`${API}/run`, { assembly })
      const data = res.data
      setOutput(data.output ?? '')
      setRunOk(data.success)
      setStatus({
        type: data.success ? 'ok' : 'error',
        text: data.success ? 'Ejecución exitosa' : 'Programa terminó con error',
      })
      setActiveTab('output')
    } catch (err) {
      const msg = err.response?.data?.detail || err.message
      setOutput('Error de conexión: ' + msg)
      setRunOk(false)
      setStatus({ type: 'error', text: 'Error de conexión' })
      setActiveTab('output')
    } finally {
      setLoading(false)
    }
  }, [assembly, loading])

  // --- Limpiar ---
  const handleClear = () => {
    setCode(EXAMPLE_CODE)
    setAssembly('')
    setAst(null)
    setOutput(null)
    setRunOk(null)
    setMessages('')
    setStatus({ type: 'idle', text: 'Listo' })
  }

  // --- Tabs ---
  const tabs = [
    { id: 'assembly', label: '⚙️  Ensamblador x86', badge: assembly ? '' : null },
    { id: 'ast',      label: '🌳  AST',             badge: ast     ? '' : null },
    { id: 'output',   label: '▶️  Salida',           badge: output !== null ? '' : null },
    { id: 'messages', label: '📋  Mensajes',         badge: messages ? '!' : null },
  ]

  return (
    <div className="app">
      {/* ------------------------------------------------------------------ */}
      {/* Header                                                               */}
      {/* ------------------------------------------------------------------ */}
      <header className="header">
        <div className="header-left">
          <span className="logo">⚙️</span>
          <span className="title">CS# Compiler IDE</span>
          <span className="subtitle">Compilador x86-64</span>
        </div>

        <div className="toolbar">
          <button
            className="btn btn-compile"
            onClick={handleCompile}
            disabled={loading}
          >
            {loading ? '⏳' : '▶'} Compilar
          </button>
          <button
            className="btn btn-run"
            onClick={handleRun}
            disabled={!assembly || loading}
            title={!assembly ? 'Compila primero' : 'Ejecutar programa'}
          >
            🚀 Ejecutar
          </button>
          <button
            className="btn btn-clear"
            onClick={handleClear}
            disabled={loading}
          >
            ↺ Reset
          </button>
        </div>

        <div className={`status-bar status-${status.type}`}>
          {status.type === 'loading' && <span className="spinner" />}
          {status.text}
        </div>
      </header>

      {/* ------------------------------------------------------------------ */}
      {/* Layout principal                                                     */}
      {/* ------------------------------------------------------------------ */}
      <div className="layout">
        {/* Panel izquierdo: Editor */}
        <div className="editor-panel">
          <div className="panel-header">
            <span>📝 Código Fuente</span>
            <span className="panel-hint">C# subset</span>
          </div>
          <div className="editor-wrap">
            <Editor
              height="100%"
              defaultLanguage="csharp"
              value={code}
              onChange={v => setCode(v ?? '')}
              theme="vs-dark"
              options={{
                fontSize: 14,
                minimap: { enabled: false },
                scrollBeyondLastLine: false,
                wordWrap: 'on',
                lineNumbers: 'on',
                renderLineHighlight: 'all',
                fontFamily: "'Cascadia Code', 'Fira Code', 'Consolas', monospace",
                fontLigatures: true,
              }}
            />
          </div>
        </div>

        {/* Divisor */}
        <div className="divider" />

        {/* Panel derecho: Tabs */}
        <div className="output-panel">
          <div className="tabs">
            {tabs.map(tab => (
              <button
                key={tab.id}
                className={`tab ${activeTab === tab.id ? 'tab-active' : ''}`}
                onClick={() => setActiveTab(tab.id)}
              >
                {tab.label}
                {tab.badge === '!' && <span className="tab-badge">!</span>}
              </button>
            ))}
          </div>

          <div className="tab-content">
            {activeTab === 'assembly' && <AssemblyPanel assembly={assembly} />}
            {activeTab === 'ast'      && <ASTViewer ast={ast} />}
            {activeTab === 'output'   && <OutputPanel output={output} success={runOk} />}
            {activeTab === 'messages' && (
              <div className="messages-panel">
                {messages
                  ? <pre className="messages-text">{messages}</pre>
                  : <div className="empty-state">
                      <div className="empty-icon">📋</div>
                      <p>Sin mensajes</p>
                    </div>
                }
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  )
}
