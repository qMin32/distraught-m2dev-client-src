param(
  [string]$Pattern = "itemdesc.txt",
  [string]$OutFile = "itemdesc.json"
)

$result = @{}

Get-ChildItem -Directory | ForEach-Object {

  $lang = $_.Name
  $file = Join-Path $_.FullName $Pattern

  if (-not (Test-Path $file)) { return }

  Write-Host "Reading $lang/itemdesc.txt"

  Get-Content -LiteralPath $file -Encoding UTF8 | ForEach-Object {

	$line = $_
	if ([string]::IsNullOrWhiteSpace($line)) { return }

	$parts = $line -split "`t", 3

	$vnum = $parts[0]
	$name = ""
	$desc = ""

	if ($parts.Length -ge 2) { $name = $parts[1].Trim() }
	if ($parts.Length -ge 3) { $desc = $parts[2].Trim() }

	if ([string]::IsNullOrWhiteSpace($name) -and [string]::IsNullOrWhiteSpace($desc)) {
		return
	}
	
	if (-not $result.ContainsKey($vnum)) {
		$result[$vnum] = @{}
	}
	
	$result[$vnum][$lang] = @($name, $desc)
  }
}

# min 3 langs
$minLangs = 3

$filtered = @{}
foreach ($vnum in $result.Keys) {
	$langCount = $result[$vnum].Keys.Count
	if ($langCount -ge $minLangs) {
		$filtered[$vnum] = $result[$vnum]
	}
}

$ordered = [ordered]@{}
$filtered.Keys | Sort-Object {[int]$_} | ForEach-Object {
	$ordered[$_] = $filtered[$_]
}

$json = $ordered | ConvertTo-Json -Depth 10

$json = $json -replace '\[\s*"(.*?)"\s*,\s*"(.*?)"\s*\]', '["$1","$2"]'
$json = $json -replace '\\u0027', "'"

# UTF8 fara BOM
$utf8 = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($OutFile, $json, $utf8)

Write-Host "Wrote $OutFile"