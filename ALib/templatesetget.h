//
//  templatesetget.h
//  ALib
//
//  Created by Giuseppe Coppini on 27/02/26.
//

protected:
    <#Type#> p_<#variableName#>;

public:
<#Type#> <#variableName#>() const noexcept { return p_<#variableName#>}
       

<#Type#> set<#CapitalizedName#>(<#Type#> <#variableName#>Value;)  {if(!validValue(<#variableName#>Value)) hrow std::invalid_argument("<#CapitalizedName#> Value not valid"); p_<#variableName#> = <#variableName#>Value;}
    }

