
$a = ls ..\..\..\..\..\3rd_party\EASTL\include\EASTL
$as = $a | %{$_.Name}| ?{$_ -match ".h" }|%{$_.substring(0, $_.length-2)}
$t = "#pragma once

#ifdef USE_STDSTL
#include <__NAME__>
#else /*use EASTL*/
#include <EASTL\__NAME__.h>
#endif
"

$as | % {
  $n = $_;
  $d = $t.Replace("__NAME__", $_);
  $d | Out-File -Encoding utf8 $n;
}