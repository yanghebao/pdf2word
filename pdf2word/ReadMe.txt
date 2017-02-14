========================================================================
    WIN32 应用程序：pdf2word 项目概述
========================================================================

人工服务接口
1、上传文件 http://upload.pdf00.com/php/controller.php?action=uploadfile&encode=utf-8&time=21345646465&check=md5("time|&#^#")   
			upfile:data 
			
	检查文件 http://upload.pdf00.com/files/check_fileext?md5=5BCFA648233E25F8DEF3F4C471ABD7A6
	
2、支付 http://www.pdfpay.com:8080/pay/?viptype=3&filemd5=ASDFEI2AL&page=5&email=test@a.net&phone=13588129412&time=21345646465&check=md5("page|filemd5|time|&#^#")&filename=