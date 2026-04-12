param(
  [string]$JsonFile = "itemdesc.json",
  [string]$OutRoot = "out"
)

$jsonText = Get-Content $JsonFile -Raw -Encoding UTF8
$data = $jsonText | ConvertFrom-Json

# colectează limbile
$langs = @()

foreach ($p in $data.PSObject.Properties) {
    foreach ($lp in $p.Value.PSObject.Properties) {
        if ($langs -notcontains $lp.Name) {
            $langs += $lp.Name
        }
    }
}

if ($langs.Count -eq 0) {
    Write-Host "No languages found in JSON."
    exit
}

# sortează vnum
$vnums = $data.PSObject.Properties.Name | Sort-Object {[int]$_}

foreach ($lang in $langs) {

    $dir = Join-Path $OutRoot $lang
    New-Item -ItemType Directory -Path $dir -Force | Out-Null

    $outFile = Join-Path $dir "itemdesc.txt"

    $lines = @()

    foreach ($vnum in $vnums) {

        $entry = $data.$vnum
        $arr = $entry.$lang

        if ($null -eq $arr) { continue }

        $name = ""
        $desc = ""

        if ($arr.Count -ge 1) { $name = ([string]$arr[0]).Trim() }
		if ($arr.Count -ge 2) { $desc = ([string]$arr[1]).Trim() }
		
		if ([string]::IsNullOrWhiteSpace($name)) { continue }
		
		if ([string]::IsNullOrWhiteSpace($desc)) {
			$lines += "$vnum`t$name"
		}
		else {
			$lines += "$vnum`t$name`t$desc"
		}
    }

    $utf8 = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllLines($outFile, $lines, $utf8)

    Write-Host "Wrote $outFile"
}

Write-Host "Done."