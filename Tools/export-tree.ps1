# Exporta a estrutura de diretórios do projeto para um arquivo de texto

# Diretório raiz do projeto
$root = (Get-Location).Path

# Arquivo de saída
$output = Join-Path $root "estrutura.txt"

# Diretórios que serão ignorados
$exclude = @(
    "build",
    "out",
    ".git",
    "vcpkg_installed",
    "cmake-build-debug",
    "cmake-build-release",
    ".vs"
)

# Função recursiva para gerar a árvore
function Show-Tree {
    param (
        [string]$Path,
        [string]$Prefix = ""
    )

    # Obtém diretórios e arquivos, ignorando diretórios excluídos
    $items = Get-ChildItem -LiteralPath $Path |
        Where-Object {
            $_.PSIsContainer -eq $false -or
            $exclude -notcontains $_.Name
        } |
        Sort-Object @{Expression = { -not $_.PSIsContainer }}, Name

    for ($i = 0; $i -lt $items.Count; $i++) {

        $item = $items[$i]
        $isLast = ($i -eq $items.Count - 1)

        if ($isLast) {
            $connector = "\--- "
            $newPrefix = $Prefix + "    "
        }
        else {
            $connector = "+--- "
            $newPrefix = $Prefix + "|   "
        }

        $script:lines.Add($Prefix + $connector + $item.Name)

        if ($item.PSIsContainer) {
            Show-Tree -Path $item.FullName -Prefix $newPrefix
        }
    }
}

# Lista de linhas
$script:lines = [System.Collections.Generic.List[string]]::new()

# Cabeçalho
$lines.Add((Split-Path $root -Leaf))
$lines.Add("")

# Gera a árvore
Show-Tree -Path $root

# Salva em UTF-8
$lines | Set-Content -Path $output -Encoding UTF8

Write-Host "Árvore gerada com sucesso!"
Write-Host "Arquivo: $output"
