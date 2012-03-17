function wizInit() {
	edit_x = document.getElementById("edit-x");
	unit_x = "mm";
	edit_y = document.getElementById("edit-y");
	unit_y = "mm";
	edit_width = document.getElementById("edit-width");
	unit_width = "mm";
	edit_height = document.getElementById("edit-height");
	unit_height = "mm";
	edit_file = document.getElementById("edit-file");
	check_entire = document.getElementById("check-entire");
	check_rotation = document.getElementById("check-rotation");
	edit_rotation = document.getElementById("edit-rotation");
	label_rotation = document.getElementById("label-rotation");
	
	edit_x.focus();
}

function searchFile() {
	var nsIFilePicker = Components.interfaces.nsIFilePicker;
	var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
	fp.init(window, "Sélectionner un fichier", nsIFilePicker.modeOpen);
	fp.appendFilter("Images", "*.jpg; *.jpeg; *.gif; *.png");
	fp.appendFilter("Tous", "*.*");

	var res = fp.show();
	if (res != nsIFilePicker.returnOK)
		return (false);

	var thefile = fp.file;
	if (thefile.exists() == false)
		return (false);

	edit_file.value = thefile.path;
}

function checkRotation() {
	if (check_rotation.checked) {
		edit_rotation.disabled = false;
		edit_rotation.focus();
	} else {
		edit_rotation.disabled = true;
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
	} else if (!edit_height.value.match(/^\d+$/) &&
		   !edit_height.value.match(/^\d+\.\d+$/)) {
		alert("Mauvaise valeur de hauteur");
		return;
	} else if (!edit_file.value.length) {
		alert("Mauvaise nom de fichier");
		return;
	} else if (check_rotation.checked &&
		   !edit_rotation.value.match(/^-?\d+$/) &&
		   !edit_linewidth.value.match(/^-?\d+\.\d+$/)) {
		alert("Mauvaise valeur de rotation");
		return;
	}
	var str = "<image x=\"" + xmlEncode(edit_x.value) + unit_x + "\" ";
	str += "y=\"" + xmlEncode(edit_y.value) + unit_y + "\" ";
	str += "width=\"" + xmlEncode(edit_width.value) + unit_width + "\" ";
	str += "height=\"" + xmlEncode(edit_height.value) + unit_height + "\" ";
	str += "file=\"" + xmlEncode(edit_file.value) + "\"";
	if (check_rotation.checked)
		str += " rotation=\"" + xmlEncode(edit_rotation.value) + "\"";
	if (check_entire.checked)
		str += " entire=\"YES\"";
	str += "/>";
	window.opener.carta.insert(str, 0);
	window.close();
}
