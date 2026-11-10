# Exporta o código-fonte do projeto para um único arquivo de texto, 
# ignorando diretórios específicos e incluindo apenas arquivos com extensões desejadas.

# Diretório raiz do projeto
$root = (Get-Location).Path

# Arquivo de saída
$output = Join-Path $root "codigo-fonte-completo.txt"

# Diretórios que serão ignorados
$exclude = @(
    "build",
    "out",
    ".git",
    "vcpkg_installed",
    "cmake-build-debug",
    "cmake-build-release"
)

# Extensões desejadas
$extensions = @(".hpp", ".cpp")

# Função para percorrer os diretórios
function Get-SourceFiles($directory) {

    # Arquivos do diretório atual
    Get-ChildItem -LiteralPath $directory -File |
        Where-Object { $extensions -contains $_.Extension }

    # Percorre subdiretórios, ignorando os excluídos
    Get-ChildItem -LiteralPath $directory -Directory |
        Where-Object { $exclude -notcontains $_.Name } |
        ForEach-Object {
            Get-SourceFiles $_.FullName
        }
}

# Coleta os arquivos
$files = @(Get-SourceFiles $root | Sort-Object FullName)

# Cria o arquivo de saída
$writer = [System.IO.StreamWriter]::new(
    $output,
    $false,
    [System.Text.UTF8Encoding]::new($false)
)

try {
    foreach ($file in $files) {

        # Caminho relativo ao projeto
        $relativePath = $file.FullName.Substring(
            $root.Length
        ).TrimStart('\', '/')

        # Cabeçalho de identificação
        $writer.WriteLine("")
        $writer.WriteLine("// ==================================================")
        $writer.WriteLine("// Arquivo: $relativePath")
        $writer.WriteLine("// ==================================================")
        $writer.WriteLine("")

        # Conteúdo original
        $content = [System.IO.File]::ReadAllText($file.FullName)
        $writer.WriteLine($content)
        $writer.WriteLine("")
    }
}
finally {
    $writer.Dispose()
}

Write-Host "Exportação concluída!"
Write-Host "Arquivos encontrados: $($files.Count)"
Write-Host "Resultado: $output"
