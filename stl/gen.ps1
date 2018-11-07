
function generate_include_files()
{
    $path =  '.\EASTL'
    $names = (ls $path) | %{$_.Name}| ?{$_ -match ".h" }|%{$_.substring(0, $_.length-2)}
    $content = @"

#ifndef __pkn_stl___NAME__
#define __pkn_stl___NAME__

#pragma once

#include "wrap/config.h"

#if !defined(PKN_USE_STDSTL) && !defined(PKN_USE_EASTL)
#define PKN_USE_STDSTL
#endif

#ifdef PKN_USE_STDSTL

#  include <__NAME__>
#  ifndef stl
#    define stl std
#  endif
//namespace stl
//{
//    using namespace std;
//}

#else /*use EASTL*/

#  include "./EASTL/__NAME__.h"
#  ifndef stl
#    define stl eastl
#  endif
//namespace stl
//{
//    using namespace eastl;
//}

#endif


#endif /*__pkn_stl___NAME__*/
"@

    $res = $names | % {
      $name = $_;
      $data = $content.Replace("__NAME__", $name);
      $data | Out-File -Encoding utf8 $name;
      Write-Host $name;
      $name
    }
    echo ----------------------------
    echo finished!
    $n = $res.Count
    echo "$n files generated!"
    sleep 0.5
}

function fix_abs_include($fs_root, $include_abs_root, $deep=1)
{
    $relative_root = ("../" * $deep)
    if(-not $relative_root){
        $relative_root = './'
    }
    $pattern = @"
<($include_abs_root)[\\/]([^>]*)>
"@
    $replacement = "`"$relative_root`$1/`$2`""

    echo "root : $fs_root"
    echo "deep : $deep"
    echo "include_abs_root : $include_abs_root"
    echo "relative_root : $relative_root"
    echo "pattern : $pattern"
    echo "replacement : $replacement"

    ls $fs_root | % {
        $_ | ?{$_.Name -match ".h|.cpp|.hpp"} | % {
            $file = $_.FullName;
            $data = Get-Content $file -Encoding UTF8 -Raw
            $data = $data -replace $pattern, $replacement
            $data | Out-File -FilePath $file -Encoding utf8 
            Write-Host $file
        }
        
        $_ | ? { $_.Attributes -eq [System.IO.FileAttributes]::Directory } | % {
            $dirname = $_.FullName
            fix_abs_include $dirname -deep ($deep+1) -include_abs_root $include_abs_root
        }
    }
}

clear;
fix_abs_include '.\EABase' -include_abs_root 'EABase|EASTL'
fix_abs_include '.\EASTL' -include_abs_root 'EABase|EASTL'
fix_abs_include '.\wrap\user_mode' -include_abs_root 'EABase|EASTL' -deep 2

generate_include_files