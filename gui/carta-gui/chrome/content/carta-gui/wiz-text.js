function wizInit() {
	font = "Courier";
	edit_size = document.getElementById("edit-size");
	unit_size = "pt";
	edit_color = document.getElementById("edit-color");
	align = "left";
	edit_space = document.getElementById("edit-space");
	check_underline = document.getElementById("check-underline");
	check_overline = document.getElementById("check-overline");
	check_strikeout = document.getElementById("check-strikeout");
	edit_x = document.getElementById("edit-x");
	unit_x = "mm";
	edit_y = document.getElementById("edit-y");
	unit_y = "mm";
	edit_width = document.getElementById("edit-width");
	unit_width = "mm";
	edit_height = document.getElementById("edit-height");
	unit_height = "mm";
	check_adapt = document.getElementById("check-adapt");
	check_charspace = document.getElementById("check-charspace");
	edit_charspace = document.getElementById("edit-charspace");
	unit_charspace = "mm";
	check_hscale = document.getElementById("check-hscale");
	edit_hscale = document.getElementById("edit-hscale");
	check_border = document.getElementById("check-border");
	edit_border = document.getElementById("edit-border");
	unit_border = "mm";
	check_radius = document.getElementById("check-radius");
	edit_radius = document.getElementById("edit-radius");
	unit_radius = "mm";
	check_linewidth = document.getElementById("check-linewidth");
	edit_linewidth = document.getElementById("edit-linewidth");
	unit_linewidth = "mm";
	check_linecolor = document.getElementById("check-linecolor");
	edit_linecolor = document.getElementById("edit-linecolor");
	check_fillcolor = document.getElementById("check-fillcolor");
	edit_fillcolor = document.getElementById("edit-fillcolor");
	check_dash = document.getElementById("check-dash");
	edit_dash1 = document.getElementById("edit-dash1");
	unit_dash1 = "mm";
	edit_dash2 = document.getElementById("edit-dash2");
	unit_dash2 = "mm";
	check_opacity = document.getElementById("check-opacity");
	edit_opacity = document.getElementById("edit-opacity");
	check_blendmode = document.getElementById("check-blendmode");
	blendmode = "Multiply";
	check_rotation = document.getElementById("check-rotation");
	edit_rotation = document.getElementById("edit-rotation");
	edit_text = document.getElementById("edit-text");

	menulist_charspace = document.getElementById("menulist-charspace");
	menulist_border = document.getElementById("menulist-border");
	menulist_radius = document.getElementById("menulist-radius");	
	menulist_linewidth = document.getElementById("menulist-linewidth");
	menulist_dash1 = document.getElementById("menulist-dash1");
	menulist_dash2 = document.getElementById("menulist-dash2");
	menulist_blendmode = document.getElementById("menulist-blendmode");
	
	edit_x.focus();
}

function checkRotation() {
	if (check_rotation.checked) {
		edit_rotation.disabled = false;
		edit_rotation.focus();
	} else {
		edit_rotation.disabled = true;
	}
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

function checkRadius() {
	if (check_radius.checked) {
		edit_radius.disabled = false;
		menulist_radius.disabled = false;
		edit_radius.focus();
	} else {
		edit_radius.disabled = true;
		menulist_radius.disabled = true;
	}
}

function checkBorder() {
	if (check_border.checked) {
		edit_border.disabled = false;
		menulist_border.disabled = false;
		edit_border.focus();
	} else {
		edit_border.disabled = true;
		menulist_border.disabled = true;
	}
}

function checkHscale() {
	if (check_hscale.checked) {
		edit_hscale.disabled = false;
		edit_hscale.focus();
	} else {
		edit_hscale.disabled = true;
	}
}

function checkCharSpace() {
	if (check_charspace.checked) {
		edit_charspace.disabled = false;
		menulist_charspace.disabled = false;
		edit_charspace.focus();
	} else {
		edit_charspace.disabled = true;
		menulist_charspace.disabled = true;
	}
}

function wizValid() {
	if (!edit_size.value.match(/^\d+$/) &&
	    !edit_size.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de taille");
		return;
	} else if (!edit_color.value.match(/^\#[a-fA-F0-9]{6}$/) &&
		   !edit_color.value.match(/^\$\w+$/)) {
		alert("Mauvaise valeur de couleur");
		return;
	} else if (!edit_space.value.match(/^\d+$/) &&
		   !edit_space.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur d'interlignage");
		return;
	} else if (!edit_x.value.match(/^-?\d+$/) &&
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
	} else if (!edit_height.value.match(/^\d+$/) &&
		   !edit_height.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de hauteur");
		return;
	} else if (check_charspace.checked &&
		   !edit_charspace.value.match(/^-?\d+$/) &&
		   !edit_charspace.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur d'espacement entre les caract�res");
		return;
	} else if (!edit_hscale.value.match(/^\d+$/) &&
		   !edit_hscale.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur d'�chelle horizontale");
		return;
	} else if (check_border.checked &&
		   !edit_border.value.match(/^\d+$/) &&
		   !edit_border.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de bordure");
		return;
	} else if (check_radius.checked &&
		   !edit_radius.value.match(/^\d+$/) &&
		   !edit_radius.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de rayon");
		return;
	} else if (check_linewidth.checked &&
		   !edit_linewidth.value.match(/^\d+$/) &&
		   !edit_linewidth.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur d'�paisseur de ligne");
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
	} else if (check_dash.checked &&
		   !edit_dash1.value.match(/^\d+$/) &&
		   !edit_dash1.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de pointill� 1");
		return;
	} else if (check_dash.checked &&
		   !edit_dash2.value.match(/^\d+$/) &&
		   !edit_dash2.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de pointill� 2");
		return;
	} else if (check_opacity.checked &&
		   (!edit_opacity.value.match(/^\d+$/) &&
		    !edit_opacity.value.match(/^\d+\.\d+$/)) ||
		   (parseFloat(edit_opacity.value) < 0 || parseFloat(edit_opacity.value) > 1)) {
		alert("Mauvaise valeur d'opacit�");
		return;
	} else if (check_rotation.checked &&
		   !edit_rotation.value.match(/^-?\d+$/) &&
		   !edit_rotation.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de rotation");
		return;
	}
	
	var str = "<text font=\"" + xmlEncode(font) + "\" ";
	str += "size=\"" + xmlEncode(edit_size.value) + unit_size + "\" ";
	str += "color=\"" + xmlEncode(edit_color.value) + "\" ";
	str += "align=\"" + xmlEncode(align) + "\" ";
	str += "space=\"" + xmlEncode(edit_space.value) + "\" ";
	str += "x=\"" + xmlEncode(edit_x.value) + unit_x + "\" ";
	str += "y=\"" + xmlEncode(edit_y.value) + unit_y + "\" ";
	str += "width=\"" + xmlEncode(edit_width.value) + unit_width + "\" ";
	str += "height=\"" + xmlEncode(edit_height.value) + unit_height + "\"";
	if (check_radius.checked)
		str += " radius=\"" + xmlEncode(edit_radius.value) + unit_radius + "\"";
	if (check_adapt.checked)
		str += " adapt=\"YES\"";
	if (check_rotation.checked)
		str += " rotation=\"" + xmlEncode(edit_rotation.value) + "\"";
	if (check_linewidth.checked)
		str += " line-width=\"" + xmlEncode(edit_linewidth.value) + unit_linewidth + "\"";
	if (check_linecolor.checked)
		str += " line-color=\"" + xmlEncode(edit_linecolor.value) + "\"";
	if (check_fillcolor.checked)
		str += " fill-color=\"" + xmlEncode(edit_fillcolor.value) + "\"";
	if (check_dash.checked) {
		str += " dash1=\"" + xmlEncode(edit_dash1.value) + unit_dash1 + "\"";
		str += " dash2=\"" + xmlEncode(edit_dash2.value) + unit_dash2 + "\"";
	}
	if (check_opacity.checked)
		str += " opacity=\"" + xmlEncode(edit_opacity.value) + "\"";
	if (check_blendmode.checked)
		str += " blenmode=\"" + blendmode + "\"";
	if (check_border.checked)
		str += " border=\"" + xmlEncode(edit_border.value) + unit_border + "\"";
	if (check_underline.checked)
		str += " underline=\"YES\"";
	if (check_overline.checked)
		str += " overline=\"YES\"";
	if (check_strikeout.checked)
		str += " strikeout=\"YES\"";
	if (check_charspace.checked)
		str += " char-space=\"" + xmlEncode(edit_charspace.value) + unit_charspace + "\"";
	if (check_hscale.checked) {
		str += " h-scale=\"" + xmlEncode(edit_hscale.value) + "\"";
	}
	str += ">\n" + edit_text.value + "\n</text>";
	window.opener.carta.insert(str, 0);
	window.close();
}
