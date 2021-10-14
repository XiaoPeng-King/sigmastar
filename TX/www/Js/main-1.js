var hostip =window.location.host;

//var txName = ['','','','','','','','','','','','','','','','','','','','','','','',''];
var rxName = ['','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','',''];
var rxMulticast = ['','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','','',''];
var num = ['001','002','003','004','005','006','007','008','009','010','011','012','013','014','015','016','017','018','019','020','021','022','023','024','025','026','027','028','029','030','031','032','033','034','035','036','037','038','039','040','041','042','043','044','045','046','047','048','049','050','051','052','053','054','055','056','057','058','059','060','061','062','063','064','065','066','067','068','069','070','071','072','073','074','075','076','077','078','079','080','081','082','083','084','085','086','087','088','089','090','091','092','093','094','095','096','097','098','099','100','101','102','103','104','105','106','107','108','109','110','111','112','113','114','115','116','117','118','119','120','121','122','123','124','125','126','127','128','129'];
var txName = ['','','','','','','','','','','','','','','','','','','','','','','',''];
var rxId = ['rxInput001','rxInput002','rxInput003','rxInput004','rxInput005','rxInput006','rxInput007','rxInput008','rxInput009','rxInput010','rxInput011','rxInput012','rxInput013','rxInput014','rxInput015','rxInput016','rxInput017','rxInput018','rxInput019','rxInput020','rxInput021','rxInput022','rxInput023','rxInput024','rxInput025','rxInput026','rxInput027','rxInput028','rxInput029','rxInput030','rxInput031','rxInput032','rxInput033','rxInput034','rxInput035','rxInput036','rxInput037','rxInput038','rxInput039','rxInput040','rxInput041','rxInput042','rxInput043','rxInput044','rxInput045','rxInput046','rxInput047','rxInput048','rxInput049','rxInput050','rxInput051','rxInput052','rxInput053','rxInput054','rxInput055','rxInput056','rxInput057','rxInput058','rxInput059','rxInput060','rxInput061','rxInput062','rxInput063','rxInput064','rxInput065','rxInput066','rxInput067','rxInput068','rxInput069','rxInput070','rxInput071','rxInput072','rxInput073','rxInput074','rxInput075','rxInput076','rxInput077','rxInput078','rxInput079','rxInput080','rxInput081','rxInput082','rxInput083','rxInput084','rxInput085','rxInput086','rxInput087','rxInput088','rxInput089','rxInput090','rxInput091','rxInput092','rxInput093','rxInput094','rxInput095','rxInput096','rxInput097','rxInput098','rxInput099','rxInput100','rxInput101','rxInput102','rxInput103','rxInput104','rxInput105','rxInput106','rxInput107','rxInput108','rxInput109','rxInput110','rxInput111','rxInput112','rxInput113','rxInput114','rxInput115','rxInput116','rxInput117','rxInput118','rxInput119','rxInput120','rxInput121','rxInput122','rxInput123','rxInput124','rxInput125','rxInput126','rxInput127','rxInput128','rxInput129'];
var num1 = ['1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24'];
var sceneName = ['', '', '', '', ''];

//button html
/*
function LoopButton()
{
	for (var i=1; i<=128; i++)
	{
		//document.write("<div><button class=\"Select_btn\">button</button></div>");
	}
}
*/


function LoopButton()
{
	for (var i=0; i<=127; i++)
	{
		var tiganStr = $("#selectTag").html();
		tiganStr = delHtmlTags(tiganStr, "{{num}}", num[i]);
		$(".addSelectRx").append(tiganStr);
	}
}

function delHtmlTags(str, reallyDo, replaceWith)
{
	var e = new RegExp(reallyDo, "g");
	words = str.replace(e, replaceWith);

	return words;
}


//window.onload = 
function selectBand()
{
	for (var i=0; i<=127; i++)
	{
		var numStr = "selectRX" + num[i].toString();
		var sel = document.getElementById(numStr);
		if (sel&&sel.addEventListener)
		{
			sel.addEventListener('change',function(e)
			{
				var ev = e||window.event;
				var target = ev.target||ev.srcElement;
				
				//alert(target.id);
				
				//document.getElementById("rxInput001").value = target.value;
				
				GetRxMulticast(target.id);
			},false)
		}
	}
}

function SetRXMulticast()
{
	var rxSelectId = ['selectRX001','selectRX002','selectRX003','selectRX004','selectRX005','selectRX006','selectRX007','selectRX008','selectRX009','selectRX010','selectRX011','selectRX012','selectRX013','selectRX014','selectRX015','selectRX016','selectRX017','selectRX018','selectRX019','selectRX020','selectRX021','selectRX022','selectRX023','selectRX024','selectRX025','selectRX026','selectRX027','selectRX028','selectRX029','selectRX030','selectRX031','selectRX032','selectRX033','selectRX034','selectRX035','selectRX036','selectRX037','selectRX038','selectRX039','selectRX040','selectRX041','selectRX042','selectRX043','selectRX044','selectRX045','selectRX046','selectRX047','selectRX048','selectRX049','selectRX050','selectRX051','selectRX052','selectRX053','selectRX054','selectRX055','selectRX056','selectRX057','selectRX058','selectRX059','selectRX060','selectRX061','selectRX062','selectRX063','selectRX064','selectRX065','selectRX066','selectRX067','selectRX068','selectRX069','selectRX070','selectRX071','selectRX072','selectRX073','selectRX074','selectRX075','selectRX076','selectRX077','selectRX078','selectRX079','selectRX080','selectRX081','selectRX082','selectRX083','selectRX084','selectRX085','selectRX086','selectRX087','selectRX088','selectRX089','selectRX090','selectRX091','selectRX092','selectRX093','selectRX094','selectRX095','selectRX096','selectRX097','selectRX098','selectRX099','selectRX100','selectRX101','selectRX102','selectRX103','selectRX104','selectRX105','selectRX106','selectRX107','selectRX108','selectRX109','selectRX110','selectRX111','selectRX112','selectRX113','selectRX114','selectRX115','selectRX116','selectRX117','selectRX118','selectRX119','selectRX120','selectRX121','selectRX122','selectRX123','selectRX124','selectRX125','selectRX126','selectRX127','selectRX128','selectRX129'];
	var rxSelectValue = ['1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24'];
	var rxElement = ['rxSelect1','rxSelect2','rxSelect3','rxSelect4','rxSelect5','rxSelect6','rxSelect7','rxSelect8','rxSelect9','rxSelect10','rxSelect11','rxSelect12','rxSelect13','rxSelect14','rxSelect15','rxSelect16','rxSelect17','rxSelect18','rxSelect19','rxSelect20','rxSelect21','rxSelect22','rxSelect23','rxSelect24','rxSelect25','rxSelect26','rxSelect27','rxSelect28','rxSelect29','rxSelect30','rxSelect31','rxSelect32','rxSelect33','rxSelect34','rxSelect35','rxSelect36','rxSelect37','rxSelect38','rxSelect39','rxSelect40','rxSelect41','rxSelect42','rxSelect43','rxSelect44','rxSelect45','rxSelect46','rxSelect47','rxSelect48','rxSelect49','rxSelect50','rxSelect51','rxSelect52','rxSelect53','rxSelect54','rxSelect55','rxSelect56','rxSelect57','rxSelect58','rxSelect59','rxSelect60','rxSelect61','rxSelect62','rxSelect63','rxSelect64','rxSelect65','rxSelect66','rxSelect67','rxSelect68','rxSelect69','rxSelect70','rxSelect71','rxSelect72','rxSelect73','rxSelect74','rxSelect75','rxSelect76','rxSelect77','rxSelect78','rxSelect79','rxSelect80','rxSelect81','rxSelect82','rxSelect83','rxSelect84','rxSelect85','rxSelect86','rxSelect87','rxSelect88','rxSelect89','rxSelect90','rxSelect91','rxSelect92','rxSelect93','rxSelect94','rxSelect95','rxSelect96','rxSelect97','rxSelect98','rxSelect99','rxSelect100','rxSelect101','rxSelect102','rxSelect103','rxSelect104','rxSelect105','rxSelect106','rxSelect107','rxSelect108','rxSelect109','rxSelect110','rxSelect111','rxSelect112','rxSelect113','rxSelect114','rxSelect115','rxSelect116','rxSelect117','rxSelect118','rxSelect119','rxSelect120','rxSelect121','rxSelect122','rxSelect123','rxSelect124','rxSelect125','rxSelect126','rxSelect127','rxSelect128','rxSelect129'];

	var i;
	
	$.ajax({
		type: "GET",
		url: "http://" + hostip + "/cgi-bin/set_multicast.cgi",
		dataType:"xml",
		cache: false,
		success: function(data) {
			
			for (i=0; i<128; i++)
			{
				rxMulticast[i] = $(data).find(rxElement[i]).text();
				//$("#input_type").append(input_type);
			}
		}
	});

	for (i=0; i<128; i++)
	{
		document.getElementById(rxSelectId[i]).value = rxMulticast[i];
	}
}



function SetTxRename()
{
	var txClass = ['option01', 'option02', 'option03', 'option04', 'option05', 'option06', 'option07', 'option08', 'option09', 'option10', 'option11', 'option12', 'option13', 'option14', 'option15', 'option16', 'option17', 'option18', 'option19', 'option20', 'option21', 'option22', 'option23', 'option24' ];
	var txId = ['TX1','TX2','TX3','TX4','TX5','TX6','TX7','TX8','TX9','TX10','TX11','TX12','TX13','TX14','TX15','TX16','TX17','TX18','TX19','TX20','TX21','TX22','TX23','TX24'];
	var i, j;
	
	
	$.ajax({
		type: "GET",
		url: "http://" + hostip + "/cgi-bin/set_txRename.cgi",
		dataType:"xml",
		cache: false,
		success: function(data) {
			
			for (i=0; i<24; i++)
			{
				txName[i] = $(data).find(txId[i]).text();
				//$("#input_type").append(input_type);
			}
		}
	});
	
	for (j = 0; j<24; j++)
	{
		var obj = document.getElementsByClassName(txClass[j]);
		
		for (i = 0; i < obj.length; i++)
		{
			obj[i].text = txName[j];
		}
	}
}

function SetRxRename()
{
	//var rxId = ['rxInput001','rxInput002','rxInput003','rxInput004','rxInput005','rxInput006','rxInput007','rxInput008','rxInput009','rxInput010','rxInput011','rxInput012','rxInput013','rxInput014','rxInput015','rxInput016','rxInput017','rxInput018','rxInput019','rxInput020','rxInput021','rxInput022','rxInput023','rxInput024','rxInput025','rxInput026','rxInput027','rxInput028','rxInput029','rxInput030','rxInput031','rxInput032','rxInput033','rxInput034','rxInput035','rxInput036','rxInput037','rxInput038','rxInput039','rxInput040','rxInput041','rxInput042','rxInput043','rxInput044','rxInput045','rxInput046','rxInput047','rxInput048','rxInput049','rxInput050','rxInput051','rxInput052','rxInput053','rxInput054','rxInput055','rxInput056','rxInput057','rxInput058','rxInput059','rxInput060','rxInput061','rxInput062','rxInput063','rxInput064','rxInput065','rxInput066','rxInput067','rxInput068','rxInput069','rxInput070','rxInput071','rxInput072','rxInput073','rxInput074','rxInput075','rxInput076','rxInput077','rxInput078','rxInput079','rxInput080','rxInput081','rxInput082','rxInput083','rxInput084','rxInput085','rxInput086','rxInput087','rxInput088','rxInput089','rxInput090','rxInput091','rxInput092','rxInput093','rxInput094','rxInput095','rxInput096','rxInput097','rxInput098','rxInput099','rxInput100','rxInput101','rxInput102','rxInput103','rxInput104','rxInput105','rxInput106','rxInput107','rxInput108','rxInput109','rxInput110','rxInput111','rxInput112','rxInput113','rxInput114','rxInput115','rxInput116','rxInput117','rxInput118','rxInput119','rxInput120','rxInput121','rxInput122','rxInput123','rxInput124','rxInput125','rxInput126','rxInput127','rxInput128','rxInput129'];
	var rxElement = ['RX1','RX2','RX3','RX4','RX5','RX6','RX7','RX8','RX9','RX10','RX11','RX12','RX13','RX14','RX15','RX16','RX17','RX18','RX19','RX20','RX21','RX22','RX23','RX24','RX25','RX26','RX27','RX28','RX29','RX30','RX31','RX32','RX33','RX34','RX35','RX36','RX37','RX38','RX39','RX40','RX41','RX42','RX43','RX44','RX45','RX46','RX47','RX48','RX49','RX50','RX51','RX52','RX53','RX54','RX55','RX56','RX57','RX58','RX59','RX60','RX61','RX62','RX63','RX64','RX65','RX66','RX67','RX68','RX69','RX70','RX71','RX72','RX73','RX74','RX75','RX76','RX77','RX78','RX79','RX80','RX81','RX82','RX83','RX84','RX85','RX86','RX87','RX88','RX89','RX90','RX91','RX92','RX93','RX94','RX95','RX96','RX97','RX98','RX99','RX100','RX101','RX102','RX103','RX104','RX105','RX106','RX107','RX108','RX109','RX110','RX111','RX112','RX113','RX114','RX115','RX116','RX117','RX118','RX119','RX120','RX121','RX122','RX123','RX124','RX125','RX126','RX127','RX128','RX129'];

	var i;
	
	$.ajax({
		type: "GET",
		url: "http://" + hostip + "/cgi-bin/set_rxRename.cgi",
		dataType:"xml",
		cache: false,
		success: function(data) {
			for (i=0; i<128; i++)
			{
				rxName[i] = $(data).find(rxElement[i]).text();
				//$("#input_type").append(input_type);
			}
		}
	});
	
	for (i=0; i<128; i++)
	{
		document.getElementById(rxId[i]).value = rxName[i];
	}
}

function SetSceneName()
{
	var sceneId = ['Mode1','Mode2','Mode3','Mode4','Mode5'];
	var sceneElement = ['Scene1','Scene2','Scene3','Scene4','Scene5'];
	var i;
	
	$.ajax({
		type: "GET",
		url: "http://" + hostip + "/cgi-bin/set_sceneName.cgi",
		dataType:"xml",
		cache: false,
		success: function(data) {
			for (i=0; i<5; i++)
			{
				sceneName[i] = $(data).find(sceneElement[i]).text();
				//$("#input_type").append(input_type);
			}
		}
	});

	for (i=0; i<5; i++)
	{
		document.getElementById(sceneId[i]).value = sceneName[i];
	}

}

function GetRxMulticast(rxSelectId)
{
	var rxSelectIdNum = document.getElementById(rxSelectId);
	var rxSelectIndex = rxSelectIdNum.selectedIndex; 
	var rxSelectValue = rxSelectIdNum.options[rxSelectIndex].value;
	
	var selectId = ['selectRX001','selectRX002','selectRX003','selectRX004','selectRX005','selectRX006','selectRX007','selectRX008','selectRX009','selectRX010','selectRX011','selectRX012','selectRX013','selectRX014','selectRX015','selectRX016','selectRX017','selectRX018','selectRX019','selectRX020','selectRX021','selectRX022','selectRX023','selectRX024','selectRX025','selectRX026','selectRX027','selectRX028','selectRX029','selectRX030','selectRX031','selectRX032','selectRX033','selectRX034','selectRX035','selectRX036','selectRX037','selectRX038','selectRX039','selectRX040','selectRX041','selectRX042','selectRX043','selectRX044','selectRX045','selectRX046','selectRX047','selectRX048','selectRX049','selectRX050','selectRX051','selectRX052','selectRX053','selectRX054','selectRX055','selectRX056','selectRX057','selectRX058','selectRX059','selectRX060','selectRX061','selectRX062','selectRX063','selectRX064','selectRX065','selectRX066','selectRX067','selectRX068','selectRX069','selectRX070','selectRX071','selectRX072','selectRX073','selectRX074','selectRX075','selectRX076','selectRX077','selectRX078','selectRX079','selectRX080','selectRX081','selectRX082','selectRX083','selectRX084','selectRX085','selectRX086','selectRX087','selectRX088','selectRX089','selectRX090','selectRX091','selectRX092','selectRX093','selectRX094','selectRX095','selectRX096','selectRX097','selectRX098','selectRX099','selectRX100','selectRX101','selectRX102','selectRX103','selectRX104','selectRX105','selectRX106','selectRX107','selectRX108','selectRX109','selectRX110','selectRX111','selectRX112','selectRX113','selectRX114','selectRX115','selectRX116','selectRX117','selectRX118','selectRX119','selectRX120','selectRX121','selectRX122','selectRX123','selectRX124','selectRX125','selectRX126','selectRX127','selectRX128','selectRX129'];
	
	/*
	for (i=0;i<128;i++)
	{
		multicastId[i] = document.getElementById(rxId[i]).value;
	}
	*/
	
	urlstr = "http://"+hostip+"/cgi-bin/get_multicast.cgi?"+rxSelectId+"="+rxSelectValue;
	
	$.ajax({ 
		type:"GET",
		//type:"POST",
		url: urlstr,
		//data: dataStr,
		dataType:"text",
		cache:false,
		success:function(data)
		{
			if(data=="succeed")
			{
				alert('Setting Successful!');	
			}
			else
			{
				alert('Setting Failed!');	
			}
		}
	});
}


function GetTxName()
{
	var txClass = ['option01', 'option02', 'option03', 'option04', 'option05', 'option06', 'option07', 'option08', 'option09', 'option10', 'option11', 'option12', 'option13', 'option14', 'option15', 'option16', 'option17', 'option18', 'option19', 'option20', 'option21', 'option22', 'option23', 'option24' ];
	var txName = ['tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx1Name', 'tx21Name', 'tx22Name', 'tx23Name', 'tx1Name', 'tx24Name'];
	var txStaticName = ['TX01:', 'TX02:', 'TX03:', 'TX04:', 'TX05:', 'TX06:', 'TX07:', 'TX08:', 'TX09:', 'TX10:', 'TX11:', 'TX12:', 'TX13:', 'TX14:', 'TX15:', 'TX16:', 'TX17:', 'TX18:', 'TX19:', 'TX20:', 'TX21:', 'TX22:', 'TX23:', 'TX24:']
	
	var orderNumStr = document.getElementById("idSelectTxRename").value;
	var txInputName = document.getElementById("TxRenameInput").value;
	var orderNum = Number(orderNumStr)-1;
	var Class = txClass[orderNum];
	var name = txStaticName[orderNum] + txInputName;
	var obj = document.getElementById("myIframe").contentDocument.getElementsByClassName(Class);
	var i;
	txName[orderNum] = name;
	
	for (i = 0; i < obj.length; i++)
	{
		obj[i].text = name;
		//document.getElementById("myIframe").contentDocument.getElementsByClassName(id).text = name;
	}
	
	var urlstr="";

	//urlstr = "http://"+hostip+"/cgi-bin/set_TxName.cgi?tx1Name="+tx1Name+"&tx2Name="+tx2Name+"&tx3Name="+tx3Name+"&tx4Name="+tx4Name+"&tx5Name="+tx5Name+"&tx6Name="+tx6Name+"&tx7Name="+tx8Name+"&tx9Name="+tx9Name+"&tx10Name="+tx10Name+"&tx11Name="+tx11Name+"&tx12Name="+tx12Name+"&tx13Name="+tx13Name+"&tx14Name="+tx14Name+"&tx15Name="+tx15Name+"&tx16Name="+tx16Name+"&tx17Name="+tx17Name+"&tx18Name="+tx18Name+"&tx19Name="+tx19Name+"&tx20Name="+tx20Name+"&tx21Name="+tx21Name+"&tx22Name="+tx22Name+"&tx23Name="+tx23Name+"&tx24Name="+tx24Name;
	//urlstr = "http://"+hostip+"/cgi-bin/get_txRename.cgi?tx1Name="+txName[0]+"&tx2Name="+txName[1]+"&tx3Name="+txName[2]+"&tx4Name="+txName[3]+"&tx5Name="+txName[4]+"&tx6Name="+txName[5]+"&tx7Name="+txName[6]+"&tx8Name="+txName[7]+"&tx9Name="+txName[8]+"&tx10Name="+txName[9]+"&tx11Name="+txName[10]+"&tx12Name="+txName[11]+"&tx13Name="+txName[12]+"&tx14Name="+txName[13]+"&tx15Name="+txName[14]+"&tx16Name="+txName[15]+"&tx17Name="+txName[16]+"&tx18Name="+txName[17]+"&tx19Name="+txName[18]+"&tx20Name="+txName[19]+"&tx21Name="+txName[20]+"&tx22Name="+txName[21]+"&tx23Name="+txName[22]+"&tx24Name="+txName[23];
	urlstr = "http://"+hostip+"/cgi-bin/get_txRename.cgi?tx"+num1[orderNum]+"Name="+txName[orderNum]+"&";

	//alert(urlstr);
	$.ajax({ 
			type:"GET",
            url: urlstr,
            dataType:"text",
	        cache:false,
			success:function(data)
			{
				if(data=="succeed")
				{
					alert('Setting Successful!');	
				}
				else
				{
					alert('Setting Failed!');	
				}
			}
	    });
}

function GetRxName()
{
	//var rxId = ['rxInput001','rxInput002','rxInput003','rxInput004','rxInput005','rxInput006','rxInput007','rxInput008','rxInput009','rxInput010','rxInput011','rxInput012','rxInput013','rxInput014','rxInput015','rxInput016','rxInput017','rxInput018','rxInput019','rxInput020','rxInput021','rxInput022','rxInput023','rxInput024','rxInput025','rxInput026','rxInput027','rxInput028','rxInput029','rxInput030','rxInput031','rxInput032','rxInput033','rxInput034','rxInput035','rxInput036','rxInput037','rxInput038','rxInput039','rxInput040','rxInput041','rxInput042','rxInput043','rxInput044','rxInput045','rxInput046','rxInput047','rxInput048','rxInput049','rxInput050','rxInput051','rxInput052','rxInput053','rxInput054','rxInput055','rxInput056','rxInput057','rxInput058','rxInput059','rxInput060','rxInput061','rxInput062','rxInput063','rxInput064','rxInput065','rxInput066','rxInput067','rxInput068','rxInput069','rxInput070','rxInput071','rxInput072','rxInput073','rxInput074','rxInput075','rxInput076','rxInput077','rxInput078','rxInput079','rxInput080','rxInput081','rxInput082','rxInput083','rxInput084','rxInput085','rxInput086','rxInput087','rxInput088','rxInput089','rxInput090','rxInput091','rxInput092','rxInput093','rxInput094','rxInput095','rxInput096','rxInput097','rxInput098','rxInput099','rxInput100','rxInput101','rxInput102','rxInput103','rxInput104','rxInput105','rxInput106','rxInput107','rxInput108','rxInput109','rxInput110','rxInput111','rxInput112','rxInput113','rxInput114','rxInput115','rxInput116','rxInput117','rxInput118','rxInput119','rxInput120','rxInput121','rxInput122','rxInput123','rxInput124','rxInput125','rxInput126','rxInput127','rxInput128','rxInput129'];

	var orderNumStr = document.getElementById("idSelectRxRename").value;
	var rxInputName = document.getElementById("RxRenameInput").value;
	var orderNum = Number(orderNumStr)-1;
	var id = rxId[orderNum];
	
	rxName[orderNum] = rxInputName;
	document.getElementById("myIframe").contentDocument.getElementById(id).value = rxInputName;
	
	var urlstr= "";
	var dataStr = "";

	//urlstr = "http://"+hostip+"/cgi-bin/set_TxName.cgi?tx1Name="+tx1Name+"&tx2Name="+tx2Name+"&tx3Name="+tx3Name+"&tx4Name="+tx4Name+"&tx5Name="+tx5Name+"&tx6Name="+tx6Name+"&tx7Name="+tx8Name+"&tx9Name="+tx9Name+"&tx10Name="+tx10Name+"&tx11Name="+tx11Name+"&tx12Name="+tx12Name+"&tx13Name="+tx13Name+"&tx14Name="+tx14Name+"&tx15Name="+tx15Name+"&tx16Name="+tx16Name+"&tx17Name="+tx17Name+"&tx18Name="+tx18Name+"&tx19Name="+tx19Name+"&tx20Name="+tx20Name+"&tx21Name="+tx21Name+"&tx22Name="+tx22Name+"&tx23Name="+tx23Name+"&tx24Name="+tx24Name;
	//urlstr = "http://"+hostip+"/cgi-bin/get_rxRename.cgi?";
	/*dataStr = "rx1Name="+rxName[0]+"&rx2Name="+rxName[1]+"&rx3Name="+rxName[2]+"&rx4Name="+rxName[3]+"&rx5Name="+rxName[4]+"&rx6Name="+rxName[5]+"&rx7Name="+rxName[6]+"&rx8Name="+rxName[7]+"&rx9Name="+rxName[8]+"&rx10Name="+rxName[9]+"&rx11Name="+rxName[10]+"&rx12Name="+rxName[11]+"&rx13Name="+rxName[12]+"&rx14Name="+rxName[13]+"&rx15Name="+rxName[14]+"&rx16Name="+rxName[15]+"&rx17Name="+rxName[16]+"&rx18Name="+rxName[17]+"&rx19Name="+rxName[18]+"&rx20Name="+rxName[19]+"&rx21Name="+rxName[20]+"&rx22Name="+rxName[21]+"&rx23Name="+rxName[22]+"&rx24Name="+rxName[23]+
			"&rx25Name="+rxName[24]+"&rx26Name="+rxName[25]+"&rx27Name="+rxName[26]+"&rx28Name="+rxName[27]+"&rx29Name="+rxName[28]+"&rx30Name="+rxName[29]+"&rx31Name="+rxName[30]+"&rx32Name="+rxName[31]+"&rx33Name="+rxName[32]+"&rx34Name="+rxName[33]+"&rx35Name="+rxName[34]+"&rx36Name="+rxName[35]+"&rx37Name="+rxName[36]+"&rx38Name="+rxName[37]+"&rx39Name="+rxName[38]+"&rx40Name="+rxName[39]+"&rx41Name="+rxName[40]+"&rx42Name="+rxName[41]+"&rx43Name="+rxName[42]+"&rx44Name="+rxName[43]+"&rx45Name="+rxName[44]+"&rx46Name="+rxName[45]+"&rx47Name="+rxName[46]+"&rx48Name="+rxName[47]+
			"&rx49Name="+rxName[48]+"&rx50Name="+rxName[49]+"&rx51Name="+rxName[50]+"&rx52Name="+rxName[51]+"&rx53Name="+rxName[52]+"&rx54Name="+rxName[53]+"&rx55Name="+rxName[54]+"&rx56Name="+rxName[55]+"&rx57Name="+rxName[56]+"&rx58Name="+rxName[57]+"&rx59Name="+rxName[58]+"&rx60Name="+rxName[59]+"&rx61Name="+rxName[60]+"&rx62Name="+rxName[61]+"&rx63Name="+rxName[62]+"&rx64Name="+rxName[63]+"&rx65Name="+rxName[64]+"&rx66Name="+rxName[65]+"&rx67Name="+rxName[66]+"&rx68Name="+rxName[67]+"&rx69Name="+rxName[68]+"&rx70Name="+rxName[69]+"&rx71Name="+rxName[70]+"&rx72Name="+rxName[71]+
			"&rx73Name="+rxName[72]+"&rx74Name="+rxName[73]+"&rx75Name="+rxName[74]+"&rx76Name="+rxName[75]+"&rx77Name="+rxName[76]+"&rx78Name="+rxName[77]+"&rx79Name="+rxName[78]+"&rx80Name="+rxName[79]+"&rx81Name="+rxName[80]+"&rx82Name="+rxName[81]+"&rx83Name="+rxName[82]+"&rx84Name="+rxName[83]+"&rx85Name="+rxName[84]+"&rx86Name="+rxName[85]+"&rx87Name="+rxName[86]+"&rx88Name="+rxName[87]+"&rx89Name="+rxName[88]+"&rx90Name="+rxName[89]+"&rx91Name="+rxName[90]+"&rx92Name="+rxName[91]+"&rx93Name="+rxName[92]+"&rx94Name="+rxName[93]+"&rx95Name="+rxName[94]+"&rx96Name="+rxName[95]+
			"&rx97Name="+rxName[96]+"&rx98Name="+rxName[97]+"&rx99Name="+rxName[98]+"&rx100Name="+rxName[99]+"&rx101Name="+rxName[100]+"&rx102Name="+rxName[101]+"&rx103Name="+rxName[102]+"&rx104Name="+rxName[103]+"&rx105Name="+rxName[104]+"&rx106Name="+rxName[105]+"&rx107Name="+rxName[106]+"&rx108Name="+rxName[107]+"&rx109Name="+rxName[108]+"&rx110Name="+rxName[109]+"&rx111Name="+rxName[110]+"&rx112Name="+rxName[111]+"&rx113Name="+rxName[112]+"&rx114Name="+rxName[113]+"&rx115Name="+rxName[114]+"&rx116Name="+rxName[115]+"&rx117Name="+rxName[116]+"&rx118Name="+rxName[117]+"&rx119Name="+rxName[118]+"&rx120Name="+rxName[119]+
			"&rx121Name="+rxName[120]+"&rx122Name="+rxName[121]+"&rx123Name="+rxName[122]+"&rx124Name="+rxName[123]+"&rx125Name="+rxName[124]+"&rx126Name="+rxName[125]+"&rx127Name="+rxName[126]+"&rx128Name="+rxName[127];
	*/
	urlstr = "http://"+hostip+"/cgi-bin/get_rxRename.cgi?";
	dataStr = "rx"+num[orderNum]+"Name="+rxName[orderNum] + "&";
	
	$.ajax({ 
			//type:"GET",
            type:"POST",
            url: urlstr,
            data: dataStr,
            dataType:"text",
	        cache:false,
			success:function(data)
			{
				if(data=="succeed")
				{
					alert('Setting Successful!');	
				}
				else
				{
					alert('Setting Failed!');	
				}
			}
	    });
}

/*
function GetRxName()
{
	var rxId = ['rxInput001','rxInput002','rxInput003','rxInput004','rxInput005','rxInput006','rxInput007','rxInput008','rxInput009','rxInput010','rxInput011','rxInput012','rxInput013','rxInput014','rxInput015','rxInput016','rxInput017','rxInput018','rxInput019','rxInput020','rxInput021','rxInput022','rxInput023','rxInput024','rxInput025','rxInput026','rxInput027','rxInput028','rxInput029','rxInput030','rxInput031','rxInput032','rxInput033','rxInput034','rxInput035','rxInput036','rxInput037','rxInput038','rxInput039','rxInput040','rxInput041','rxInput042','rxInput043','rxInput044','rxInput045','rxInput046','rxInput047','rxInput048','rxInput049','rxInput050','rxInput051','rxInput052','rxInput053','rxInput054','rxInput055','rxInput056','rxInput057','rxInput058','rxInput059','rxInput060','rxInput061','rxInput062','rxInput063','rxInput064','rxInput065','rxInput066','rxInput067','rxInput068','rxInput069','rxInput070','rxInput071','rxInput072','rxInput073','rxInput074','rxInput075','rxInput076','rxInput077','rxInput078','rxInput079','rxInput080','rxInput081','rxInput082','rxInput083','rxInput084','rxInput085','rxInput086','rxInput087','rxInput088','rxInput089','rxInput090','rxInput091','rxInput092','rxInput093','rxInput094','rxInput095','rxInput096','rxInput097','rxInput098','rxInput099','rxInput100','rxInput101','rxInput102','rxInput103','rxInput104','rxInput105','rxInput106','rxInput107','rxInput108','rxInput109','rxInput110','rxInput111','rxInput112','rxInput113','rxInput114','rxInput115','rxInput116','rxInput117','rxInput118','rxInput119','rxInput120','rxInput121','rxInput122','rxInput123','rxInput124','rxInput125','rxInput126','rxInput127','rxInput128','rxInput129'];
	//var i;
	
	
	
	var orderNumStr = document.getElementById("idSelectRxRename").value;
	var rxInputName = document.getElementById("RxRenameInput").value;
	var orderNum = Number(orderNumStr);
	var id = rxId[orderNum-1];
	
	var doc = document.getElementById("myIframe").contentDocument;
	doc.getElementById(id).value = rxInputName;
	rxName[orderNum-1] = rxInputName;
	
	var urlstr="";
	var dataStr = "";

	//urlstr = "http://"+hostip+"/cgi-bin/set_TxName.cgi?tx1Name="+tx1Name+"&tx2Name="+tx2Name+"&tx3Name="+tx3Name+"&tx4Name="+tx4Name+"&tx5Name="+tx5Name+"&tx6Name="+tx6Name+"&tx7Name="+tx8Name+"&tx9Name="+tx9Name+"&tx10Name="+tx10Name+"&tx11Name="+tx11Name+"&tx12Name="+tx12Name+"&tx13Name="+tx13Name+"&tx14Name="+tx14Name+"&tx15Name="+tx15Name+"&tx16Name="+tx16Name+"&tx17Name="+tx17Name+"&tx18Name="+tx18Name+"&tx19Name="+tx19Name+"&tx20Name="+tx20Name+"&tx21Name="+tx21Name+"&tx22Name="+tx22Name+"&tx23Name="+tx23Name+"&tx24Name="+tx24Name;
	urlstr = "http://"+hostip+"/cgi-bin/get_rxRename.cgi?";
	dataStr = "rx1Name="+rxName[0]+"&rx2Name="+rxName[1]+"&rx3Name="+rxName[2]+"&rx4Name="+rxName[3]+"&rx5Name="+rxName[4]+"&rx6Name="+rxName[5]+"&rx7Name="+rxName[6]+"&rx8Name="+rxName[7]+"&rx9Name="+rxName[8]+"&rx10Name="+rxName[9]+"&rx11Name="+rxName[10]+"&rx12Name="+rxName[11]+"&rx13Name="+rxName[12]+"&rx14Name="+rxName[13]+"&rx15Name="+rxName[14]+"&rx16Name="+rxName[15]+"&rx17Name="+rxName[16]+"&rx18Name="+rxName[17]+"&rx19Name="+rxName[18]+"&rx20Name="+rxName[19]+"&rx21Name="+rxName[20]+"&rx22Name="+rxName[21]+"&rx23Name="+rxName[22]+"&rx24Name="+rxName[23]+
			"&rx25Name="+rxName[24]+"&rx26Name="+rxName[25]+"&rx27Name="+rxName[26]+"&rx28Name="+rxName[27]+"&rx29Name="+rxName[28]+"&rx30Name="+rxName[29]+"&rx31Name="+rxName[30]+"&rx32Name="+rxName[31]+"&rx33Name="+rxName[32]+"&rx34Name="+rxName[33]+"&rx35Name="+rxName[34]+"&rx36Name="+rxName[35]+"&rx37Name="+rxName[36]+"&rx38Name="+rxName[37]+"&rx39Name="+rxName[38]+"&rx40Name="+rxName[39]+"&rx41Name="+rxName[40]+"&rx42Name="+rxName[41]+"&rx43Name="+rxName[42]+"&rx44Name="+rxName[43]+"&rx45Name="+rxName[44]+"&rx46Name="+rxName[45]+"&rx47Name="+rxName[46]+"&rx48Name="+rxName[47]+
			"&rx49Name="+rxName[48]+"&rx50Name="+rxName[49]+"&rx51Name="+rxName[50]+"&rx52Name="+rxName[51]+"&rx53Name="+rxName[52]+"&rx54Name="+rxName[53]+"&rx55Name="+rxName[54]+"&rx56Name="+rxName[55]+"&rx57Name="+rxName[56]+"&rx58Name="+rxName[57]+"&rx59Name="+rxName[58]+"&rx60Name="+rxName[59]+"&rx61Name="+rxName[60]+"&rx62Name="+rxName[61]+"&rx63Name="+rxName[62]+"&rx64Name="+rxName[63]+"&rx65Name="+rxName[64]+"&rx66Name="+rxName[65]+"&rx67Name="+rxName[66]+"&rx68Name="+rxName[67]+"&rx69Name="+rxName[68]+"&rx70Name="+rxName[69]+"&rx71Name="+rxName[70]+"&rx72Name="+rxName[71]+
			"&rx73Name="+rxName[72]+"&rx74Name="+rxName[73]+"&rx75Name="+rxName[74]+"&rx76Name="+rxName[75]+"&rx77Name="+rxName[76]+"&rx78Name="+rxName[77]+"&rx79Name="+rxName[78]+"&rx80Name="+rxName[79]+"&rx81Name="+rxName[80]+"&rx82Name="+rxName[81]+"&rx83Name="+rxName[82]+"&rx84Name="+rxName[83]+"&rx85Name="+rxName[84]+"&rx86Name="+rxName[85]+"&rx87Name="+rxName[86]+"&rx88Name="+rxName[87]+"&rx89Name="+rxName[88]+"&rx90Name="+rxName[89]+"&rx91Name="+rxName[90]+"&rx92Name="+rxName[91]+"&rx93Name="+rxName[92]+"&rx94Name="+rxName[93]+"&rx95Name="+rxName[94]+"&rx96Name="+rxName[95]+
			"&rx97Name="+rxName[96]+"&rx98Name="+rxName[97]+"&rx99Name="+rxName[98]+"&rx100Name="+rxName[99]+"&rx101Name="+rxName[100]+"&rx102Name="+rxName[101]+"&rx103Name="+rxName[102]+"&rx104Name="+rxName[103]+"&rx105Name="+rxName[104]+"&rx106Name="+rxName[105]+"&rx107Name="+rxName[106]+"&rx108Name="+rxName[107]+"&rx109Name="+rxName[108]+"&rx110Name="+rxName[109]+"&rx111Name="+rxName[110]+"&rx112Name="+rxName[111]+"&rx113Name="+rxName[112]+"&rx114Name="+rxName[113]+"&rx115Name="+rxName[114]+"&rx116Name="+rxName[115]+"&rx117Name="+rxName[116]+"&rx118Name="+rxName[117]+"&rx119Name="+rxName[118]+"&rx120Name="+rxName[119]+
			"&rx121Name="+rxName[120]+"&rx122Name="+rxName[121]+"&rx123Name="+rxName[122]+"&rx124Name="+rxName[123]+"&rx125Name="+rxName[124]+"&rx126Name="+rxName[125]+"&rx127Name="+rxName[126]+"&rx128Name="+rxName[127];

	$.ajax({ 
			//type:"GET",
            type:"POST",
            url: urlstr,
            data: dataStr,
            dataType:"text",
	        cache:false,
			success:function(data)
			{
				if(data=="succeed")
				{
					alert('Setting Successful!');	
				}
				else
				{
					alert('Setting Failed!');	
				}
			}
	    });

}

*/


function rxButtonInitSet()
{
	SetRxRename();
	SetTxRename();
	SetRXMulticast();
	selectBand();
}
















