function wizInit() {
	edit_x = document.getElementById("edit-x");
	unit_x = "mm";
	edit_y = document.getElementById("edit-y");
	unit_y = "mm";
	edit_width = document.getElementById("edit-width");
	unit_width = "mm";
	check_linewidth = document.getElementById("check-linewidth");
	edit_linewidth = document.getElementById("edit-linewidth");
	unit_linewidth = "mm";
	check_linecolor = document.getElementById("check-linecolor");
	edit_linecolor = document.getElementById("edit-linecolor");
	check_fillcolor = document.getElementById("check-fillcolor");
	edit_fillcolor = document.getElementById("edit-fillcolor");
	unit_fillcolor = "mm";
	check_rotation = document.getElementById("check-rotation");
	edit_rotation = document.getElementById("edit-rotation");
	label_rotation = document.getElementById("label-rotation");
	check_dash = document.getElementById("check-dash");
	edit_dash1 = document.getElementById("edit-dash1");
	unit_dash1 = "mm";
	edit_dash2 = document.getElementById("edit-dash2");
	unit_dash2 = "mm";
	check_opacity = document.getElementById("check-opacity");
	edit_opacity = document.getElementById("edit-opacity");
	check_blendmode = document.getElementById("check-blendmode");
	blendmode = "Multiply";
	
	menulist_linewidth = document.getElementById("menulist-linewidth");
	menulist_dash1 = document.getElementById("menulist-dash1");
	menulist_dash2 = document.getElementById("menulist-dash2");
	menulist_blendmode = document.getElementById("menulist-blendmode");
	
	edit_x.focus();
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

function checkRotation() {
	if (check_rotation.checked) {
		edit_rotation.disabled = false;
		edit_rotation.focus();
		label_rotation.disabled = false;
	} else {
		edit_rotation.disabled = true;
		label_rotation.disabled = true;
	}
}

function checkFill() {
	if (check_fillcolor.checked) {
		edit_fillcolor.disabled = false;
		edit_fillcolor.focus();
	} else {
		edit_fillcolor.disabled = true;
	}
}

function checkLineWidth() {
	if (check_linewidth.checked) {
		check_linecolor.checked = true;
		edit_linewidth.disabled = false;
		menulist_linewidth.disabled = false;
		edit_linecolor.disabled = false;
		edit_linewidth.focus();
	} else {
		edit_linewidth.disabled = true;
		menulist_linewidth.disabled = true;
	}
}

function checkLineColor() {
	if (check_linecolor.checked) {
		check_linewidth.checked = true;
		edit_linewidth.disabled = false;
		menulist_linewidth.disabled = false;
		edit_linecolor.disabled = false;
		edit_linecolor.focus();
	} else {
		edit_linecolor.disabled = true;
	}
}

function wizValid() {
	if (!edit_x.value.match(/^-?\d+$/) &&
	    !edit_x.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de X");
		return;
	} else if (!edit_y.value.match(/^-?\d+$/) &&
		   !edit_y.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de Y");
		return;
	} else if (!edit_width.value.match(/^\d+$/) &&
		   !edit_width.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de largeur");
		return;
	} else if (check_linewidth.checked &&
		   !edit_linewidth.value.match(/^-?\d+$/) &&
		   !edit_linewidth.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur d'épaisseur de ligne");
		return;
	} else if (check_linecolor.checked &&
		   !edit_linecolor.value.match(/^\#[a-fA-F0-9]{6}$/) &&
		   !edit_linecolor.value.match(/^\$\w+$/)) {
		alert("Mauvaise valeur de couleur de ligne");
		return;
	} else if (check_fillcolor.checked &&
		   !edit_fillcolor.value.match(/^\#[a-fA-F0-9]{6}$/) &&
		   !edit_fillcolor.value.match(/^\$\w+$/)) {
		alert("Mauvaise valeur de couleur de remplissage");
		return;
	} else if (check_rotation.checked &&
		   !edit_rotation.value.match(/^-?\d+$/) &&
		   !edit_rotation.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de rotation");
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
	
	var str = "<hexagon x=\"" + xmlEncode(edit_x.value) + unit_x + "\" ";
	str += "y=\"" + xmlEncode(edit_y.value) + unit_y + "\" ";
	str += "width=\"" + xmlEncode(edit_width.value) + unit_width + "\"";
	if (check_linewidth.checked)
		str += " line-width=\"" + xmlEncode(edit_linewidth.value) + unit_linewidth + "\"";
	if (check_linecolor.checked)
		str += " line-color=\"" + xmlEncode(edit_linecolor.value) + "\"";
	if (check_fillcolor.checked)
		str += " fill-color=\"" + xmlEncode(edit_fillcolor.value) + "\"";
	if (check_rotation.checked)
		str += " rotation=\"" + xmlEncode(edit_rotation.value) + "\"";
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
