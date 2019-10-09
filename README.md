# PSAutoArranger
## Photoshop自动排布插件  
用于将大量相同规格图片按照指定的行列间隔排布到设定尺寸的文档中，以便于印刷出版。  
SDK版本支持Adobe Photoshsop CC 2015.5  
编译前需要设定boost库所在路径  
## 详情
包含两个工程：  
    AutoArranger：插件本身  
    TaskControl：控制台程序，用于显示处理过程以及随时中止处理。  
                 该程序需要复制到%AppData%\AutoArranger路径才能正常被调用。  
