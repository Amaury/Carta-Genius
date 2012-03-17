function wizInit() {
	edit_x1 = document.getElementById("edit-x1");
	unit_x1 = "mm";
	edit_y1 = document.getElementById("edit-y1");
	unit_y1 = "mm";
	edit_x2 = document.getElementById("edit-x2");
	unit_x2 = "mm";
	edit_y2 = document.getElementById("edit-y2");
	unit_y2 = "mm";
	edit_color = document.getElementById("edit-color");
	check_width = document.getElementById("check-width");
	edit_width = document.getElementById("edit-width");
	unit_width = "mm";
	check_dash = document.getElementById("check-dash");
	edit_dash1 = document.getElementById("edit-dash1");
	unit_dash1 = "mm";
	edit_dash2 = document.getElementById("edit-dash2");
	unit_dash2 = "mm";
	check_opacity = document.getElementById("check-opacity");
	edit_opacity = document.getElementById("edit-opacity");
	check_blendmode = document.getElementById("check-blendmode");
	blendmode = "Multiply";
	
	menulist_width = document.getElementById("menulist-width");
	menulist_dash1 = document.getElementById("menulist-dash1");
	menulist_dash2 = document.getElementById("menulist-dash2");
	menulist_blendmode = document.getElementById("menulist-blendmode");
	
	edit_x1.focus();
}

function checkBlendmode() {
	if (check_blendmode.checked) {
		menulist_blendmode.disabled = false;
	} else {
		menulist_blendmode.disabled = true;
	}
}

function checkOpacity() {
	if (check_opacity.checked) {
		edit_opacity.disabled = false;
		edit_opacity.focus();
	} else {
		edit_opacity.disabled = true;
	}
}

function checkDash() {
	if (check_dash.checked) {
		edit_dash1.disabled = false;
		edit_dash2.disabled = false;
		menulist_dash1.disabled = false;
		menulist_dash2.disabled = false;
		edit_dash1.focus();
	} else {
		edit_dash1.disabled = true;
		edit_dash2.disabled = true;
		menulist_dash1.disabled = true;
		menulist_dash2.disabled = true;
	}
}

function checkWidth() {
	if (check_width.checked) {
		edit_width.disabled = false;
		menulist_width.disabled = false;
		edit_width.focus();
	} else {
		edit_width.disabled = true;
		menulist_width.disabled = true;
	}
}

function wizValid() {
	if (!edit_x1.value.match(/^-?\d+$/) &&
	    !edit_x1.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de X1");
		return;
	} else if (!edit_y1.value.match(/^-?\d+$/) &&
		   !edit_y1.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de Y1");
		return;
	} else if (!edit_x2.value.match(/^-?\d+$/) &&
		   !edit_x2.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de X2");
		return;
	} else if (!edit_y2.value.match(/^-?\d+$/) &&
		   !edit_y2.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de Y2");
		return;
	} else if (!edit_color.value.match(/^\#[a-fA-F0-9]{6}$/) &&
		   !edit_color.value.match(/^\$\w+$/)) {
		alert("Mauvaise valeur de couleur");
		return;
	} else if (check_width.checked &&
		   !edit_width.value.match(/^-?\d+$/) &&
		   !edit_width.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur d'épaisseur de ligne");
		return;
	} else if (check_dash.checked &&
		   !edit_dash1.value.match(/^\d+$/) &&
		   !edit_dash1.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de pointillé 1");
		return;
	} else if (check_dash.checked &&
		   !edit_dash2.value.match(/^\d+$/) &&
		   !edit_dash2.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de pointillé 2");
		return;
	} else if (check_opacity.checked &&
		   (!edit_opacity.value.match(/^\d+$/) &&
		    !edit_opacity.value.match(/^\d+\.\d+$/)) ||
		   (parseFloat(edit_opacity.value) < 0 || parseFloat(edit_opacity.value) > 1)) {
		alert("Mauvaise valeur d'opacité");
		return;
	}
	
	var str = "<line x1=\"" + xmlEncode(edit_x1.value) + unit_x1 + "\" ";
	str += "y1=\"" + xmlEncode(edit_y1.value) + unit_y1 + "\" ";
	str += "x2=\"" + xmlEncode(edit_x2.value) + unit_x2 + "\" ";
	str += "y2=\"" + xmlEncode(edit_y2.value) + unit_y2 + "\" ";
	str += "color=\"" + xmlEncode(edit_color.value) + "\"";
	if (check_width.checked)
		str += " width=\"" + xmlEncode(edit_width.value) + unit_width + "\"";
	if (check_dash.checked) {
		str += " dash1=\"" + xmlEncode(edit_dash1.value) + unit_dash1 + "\"";
		str += " dash2=\"" + xmlEncode(edit_dash2.value) + unit_dash2 + "\"";
	}
	if (check_opacity.checked)
		str += " opacity=\"" + xmlEncode(edit_opacity.value) + "\"";
	if (check_blendmode.checked)
		str += " blenmode=\"" + blendmode + "\"";
	str += "/>";
	window.opener.carta.insert(str, 0);
	window.close();
}
