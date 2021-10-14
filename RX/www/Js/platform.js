var menu_name = new Array(	"href='index.html'>状态显示</A></LI>", 
							"href='SetNet.html'>网络设置</A></LI>", 
							"href='SetMedia0.html'>编码设置</A></LI>", 
							//"href='SetMedia1.html'>副流编码设置</A></LI>", 
							//"href='SetAudio.html'>音频编码设置</A></LI>", 
							"href='System.html'>系统设置</A></LI>"
						 );

var menu_nameE = new Array(	"href='indexE.html'>Status</A></LI>", 
							"href='SetNetE.html'>Network</A></LI>", 
							"href='SetMedia0E.html'>Encoder</A></LI>", 
							//"href='SetMedia1E.html'>Second stream</A></LI>", 
							//"href='SetAudioE.html'>Audio</A></LI>", 
							"href='SystemE.html'>System</A></LI>"
						 );

function showMenu(str) {
	var result = "<DIV class='navigation'><UL>";
	var bool;

	for (i=0;i<menu_name.length;i++)
	{
		result += "<LI><A ";
		bool = menu_name[i].indexOf(str);
		if (bool > 0)
			result += "class='active' ";

		result += menu_name[i];
	}
 	result += "</UL><p>高清编码器配置操作平台</p></DIV>";

	return result;
}

function showMenuE(str) {
	var result = "<DIV class='navigation'><UL>";
	var bool;

	for (i=0;i<menu_nameE.length;i++)
	{
		result += "<LI><A ";
		bool = menu_nameE[i].indexOf(str);
		if (bool > 0)
			result += "class='active' ";

		result += menu_nameE[i];
	}
 	result += "</UL><p>HD ENCODER CONFIGURATION PLATFORM</p></DIV>";

	return result;
}

function showEncodeType() {
	var result = '<select id="EncType" name="EncType" style="width:100px;" >';
	result += '<option  value="96" selected ="selected">H.264</option>';
	
	result += '</select>';
	return result;
}

function showMediaPlayExt() {
	return "";
}

function showMediaPlayExtE() {
	return "";
}

function showMediaAddrExt() {
	return "";
}

function showMediaAddrExtE() {
	return "";
}

function showResample() {
	$("#sampleshow").hide();
}

function showImgAttr() {
	$("#img_attr").hide();
}

function showIotSle() {
	$("#iot_show").show();
}

function showVideoSize(width, height, des_width, des_height) {
	var IsSlected = 0;
	var result = '<select id="VideosizeSle" name="VideosizeSle" style="width:100px;">';
	width = parseInt(width);
	var arrWidth = new Array('1920x1080', '1680x1200', '1600x900', '1440x1050', '1440x900', '1360x768', '1280x720', '1280x800', '1280x768', '1024x768', '1024x576', '960x540', '850x480', '800x600', '720x576', '720x540', '720x480', '720x404', '704x576', '640x480', '640x360','480x270');
	for (var i = 0; i < arrWidth.length; i++) {
		var str_before = arrWidth[i].split('x')[0];
		str_before = parseInt(str_before);
		var str_after = arrWidth[i].split('x')[1];
		str_after = parseInt(str_after);
		if (des_width == str_before && des_height == str_after) {
			result += '<option value="' + arrWidth[i] + '" selected ="selected">' + arrWidth[i] + '</option>';
			IsSlected = 1;
		} else {
			result += '<option value="' + arrWidth[i] + '">' + arrWidth[i] + '</option>';
		}
	}

	if (IsSlected == 0) {
		result += '<option value="0x0" selected ="selected">自动</option>';
	} else {
		result += '<option value="0x0">自动</option>';
	}
	result += '</select>';
	return result;
}

function showVideoSizeE(width, height, des_width, des_height) {
	var IsSlected = 0;
	var result = '<select id="VideosizeSle" name="VideosizeSle" style="width:100px;">';
	width = parseInt(width);
	var arrWidth = new Array('1920x1080', '1680x1200', '1600x900', '1440x1050', '1440x900', '1360x768', '1280x720', '1280x800', '1280x768', '1024x768', '1024x576', '960x540', '850x480', '800x600', '720x576', '720x540', '720x480', '720x404', '704x576', '640x480', '640x360','480x270');
	for (var i = 0; i < arrWidth.length; i++) {
		var str_before = arrWidth[i].split('x')[0];
		str_before = parseInt(str_before);
		var str_after = arrWidth[i].split('x')[1];
		str_after = parseInt(str_after);
		if (des_width == str_before && des_height == str_after) {
			result += '<option value="' + arrWidth[i] + '" selected ="selected">' + arrWidth[i] + '</option>';
			IsSlected = 1;
		} else {
			result += '<option value="' + arrWidth[i] + '">' + arrWidth[i] + '</option>';
		}
	}

	if (IsSlected == 0) {
		result += '<option value="0x0" selected ="selected">Auto</option>';
	} else {
		result += '<option value="0x0">Auto</option>';
	}
	result += '</select>';
	return result;
}
