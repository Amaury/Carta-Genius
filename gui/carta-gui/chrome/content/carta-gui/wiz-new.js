function wizInit() {
	edit_author = document.getElementById("edit-author");
	edit_title = document.getElementById("edit-title");
	edit_subject = document.getElementById("edit-subject");
	edit_keywords = document.getElementById("edit-keywords");
	edit_copyright = document.getElementById("edit-copyright");
	edit_version = document.getElementById("edit-version");
	edit_lang = document.getElementById("edit-lang");
	edit_note = document.getElementById("edit-note");
	back_side = "NONE";
	back_reverse = "NO";
	check_odd = document.getElementById("check-odd");
	check_even = document.getElementById("check-even");
	edit_odd = document.getElementById("edit-odd");
	edit_even = document.getElementById("edit-even");
	paper_format = "standard";
	paper_landscape = "NO";
	paper_type = "A4";
	edit_width = document.getElementById("edit-width");
	edit_height = document.getElementById("edit-height");
	unit_width = "mm";
	unit_height = "mm";
	margin_type = "no";
	edit_margin = document.getElementById("edit-margin");
	edit_marginwidth = document.getElementById("edit-marginwidth");
	edit_marginheight = document.getElementById("edit-marginheight");
	unit_margin = "mm";
	unit_marginwidth = "mm";
	unit_marginheight = "mm";
	card_format = "standard";
	card_landscape = "NO";
	card_type = "A7";
	edit_cardwidth = document.getElementById("edit-cardwidth");
	edit_cardheight = document.getElementById("edit-cardheight");
	unit_cardwidth = "mm";
	unit_cardheight = "mm";
	space_type = "no";
	edit_space = document.getElementById("edit-space");
	edit_spacewidth = document.getElementById("edit-spacewidth");
	edit_spaceheight = document.getElementById("edit-spaceheight");
	unit_space = "mm";
	unit_spacewidth = "mm";
	unit_spaceheight = "mm";
	
	menulist_papertype = document.getElementById("menulist-papertype");
	menulist_landscape = document.getElementById("menulist-landscape");
	menulist_width = document.getElementById("menulist-width");
	menulist_height = document.getElementById("menulist-height");
	menulist_margin = document.getElementById("menulist-margin");
	menulist_marginwidth = document.getElementById("menulist-marginwidth");
	menulist_marginheight = document.getElementById("menulist-marginheight");
	menulist_cardtype = document.getElementById("menulist-cardtype");
	menulist_cardlandscape = document.getElementById("menulist-cardlandscape");
	menulist_cardwidth = document.getElementById("menulist-cardwidth");
	menulist_cardheight = document.getElementById("menulist-cardheight");
	menulist_space = document.getElementById("menulist-space");
	menulist_spacewidth = document.getElementById("menulist-spacewidth");
	menulist_spaceheight = document.getElementById("menulist-spaceheight");
	
	edit_author.focus();
}

function checkSpace(state) {
	space_type = state;
	if (state == "no") {
		edit_space.disabled = true;
		menulist_space.disabled = true;
		edit_spacewidth.disabled = true;
		edit_spaceheight.disabled = true;
		menulist_spacewidth.disabled = true;
		menulist_spaceheight.disabled = true;
	} else if (state == "same") {
		edit_space.disabled = false;
		menulist_space.disabled = false;
		edit_spacewidth.disabled = true;
		edit_spaceheight.disabled = true;
		menulist_spacewidth.disabled = true;
		menulist_spaceheight.disabled = true;
		edit_space.focus();
	} else if (state == "special") {
		edit_space.disabled = true;
		menulist_space.disabled = true;
		edit_spacewidth.disabled = false;
		edit_spaceheight.disabled = false;
		menulist_spacewidth.disabled = false;
		menulist_spaceheight.disabled = false;
		edit_spacewidth.focus();
	}
}

function checkCard(state) {
	card_format = state;
	if (state == "standard") {
		menulist_cardtype.disabled = false;
		menulist_cardlandscape.disabled = false;
		edit_cardwidth.disabled = true;
		edit_cardheight.disabled = true;
		menulist_cardwidth.disabled = true;
		menulist_cardheight.disabled = true;
	} else if (state == "special") {
		menulist_cardtype.disabled = true;
		menulist_cardlandscape.disabled = true;
		edit_cardwidth.disabled = false;
		edit_cardheight.disabled = false;
		menulist_cardwidth.disabled = false;
		menulist_cardheight.disabled = false;
		edit_cardwidth.focus();
	}
}

function checkMargin(state) {
	margin_type = state;
	if (state == "no") {
		edit_margin.disabled = true;
		menulist_margin.disabled = true;
		edit_marginwidth.disabled = true;
		edit_marginheight.disabled = true;
		menulist_marginwidth.disabled = true;
		menulist_marginheight.disabled = true;
	} else if (state == "same") {
		edit_margin.disabled = false;
		menulist_margin.disabled = false;
		edit_marginwidth.disabled = true;
		edit_marginheight.disabled = true;
		menulist_marginwidth.disabled = true;
		menulist_marginheight.disabled = true;
		edit_margin.focus();
	} else if (state == "special") {
		edit_margin.disabled = true;
		menulist_margin.disabled = true;
		edit_marginwidth.disabled = false;
		edit_marginheight.disabled = false;
		menulist_marginwidth.disabled = false;
		menulist_marginheight.disabled = false;
		edit_marginwidth.focus();
	}
}

function checkFormat(state) {
	paper_format = state;
	if (state == "standard") {
		menulist_papertype.disabled = false;
		menulist_landscape.disabled = false;
		edit_width.disabled = true;
		edit_height.disabled = true;
		menulist_width.disabled = true;
		menulist_height.disabled = true;
	} else if (state == "special") {
		menulist_papertype.disabled = true;
		menulist_landscape.disabled = true;
		edit_width.disabled = false;
		edit_height.disabled = false;
		menulist_width.disabled = false;
		menulist_height.disabled = false;
		edit_width.focus();
	}
}

function checkOdd() {
	if (check_odd.checked) {
		edit_odd.disabled = false;
		edit_odd.focus();
	} else
		edit_odd.disabled = true;
}

function checkEven() {
	if (check_even.checked) {
		edit_even.disabled = false;
		edit_even.focus();
	} else
		edit_even.disabled = true;
}

function wizValid() {
	str = "<?xml version=\"1.0\"?>\n<pandocreon:carta-genius>\n";
	if (edit_author.value.length || edit_title.value.length || edit_subject.value.length ||
	    edit_keywords.value.length || edit_copyright.value.length || edit_version.value.length ||
	    edit_lang.value.length || edit_note.value.length) {
		str += "<info";
		if (edit_author.value.length)
			str += " author=\"" + xmlEncode(edit_author.value) + "\"";
		if (edit_title.value.length)
			str += " title=\"" + xmlEncode(edit_title.value) + "\"";
		if (edit_subject.value.length)
			str += " subject=\"" + xmlEncode(edit_subject.value) + "\"";
		if (edit_keywords.value.length)
			str += " keywords=\"" + xmlEncode(edit_keywords.value) + "\"";
		if (edit_copyright.value.length)
			str += " copyright=\"" + xmlEncode(edit_copyright.value) + "\"";
		if (edit_version.value.length)
			str += " version=\"" + xmlEncode(edit_version.value) + "\"";
		if (edit_lang.value.length)
			str += " lang=\"" + xmlEncode(edit_lang.value) + "\"";
		if (edit_note.value.length)
			str += " note=\"" + xmlEncode(edit_note.value) + "\"";
		str += "/>\n";
	}
	str += "	<back side=\"" + back_side + "\" reverse=\"" + back_reverse + "\"/>\n\
	<fonts>\n\
		<!-- ajoutez ici les définitions de fontes -->\n\
	</fonts>\n\
	<images>\n\
		<!-- ajoutez ici les déclarations d'images -->\n\
	</images>\n\
	<templates>\n\
		<!-- ajoutez ici les déclaration de templates -->\n\
	</templates>\n\
	<deck>\n";
	str += "\t\t<paper";
	if (paper_format == "standard") {
		str += " type=\"" + paper_type + "\"";
		str += " landscape=\"" + paper_landscape + "\"";
	} else {
		if (!edit_width.value.match(/^-?\d+$/) &&
		    !edit_width.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur de largeur de papier");
			return;
		}
		if (!edit_height.value.match(/^-?\d+$/) &&
		    !edit_height.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur de hauteur de papier");
			return;
		}
		str += " width=\"" + xmlEncode(edit_width.value) + unit_width + "\"";
		str += " height=\"" + xmlEncode(edit_height.value) + unit_height + "\"";
	}
	if (margin_type == "same") {
		if (!edit_margin.value.match(/^-?\d+$/) &&
		    !edit_margin.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur de marge");
			return;
		}
		str += " margin=\"" + edit_margin.value + unit_margin + "\"";
	} else if (margin_type == "special") {
		if (!edit_marginwidth.value.match(/^-?\d+$/) &&
		    !edit_marginwidth.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur de largeur de marge");
			return;
		}
		if (!edit_marginheight.value.match(/^-?\d+$/) &&
		    !edit_marginheight.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur de hauteur de marge");
			return;
		}
		str += " margin-width=\"" + edit_marginwidth.value + unit_marginwidth + "\"";
		str += " margin-height=\"" + edit_marginheight.value + unit_marginheight + "\"";
	}
	str += "/>\n";
	if (check_odd.checked || check_even.checked) {
		str += "\t\t<hidden-ditch";
		if (check_odd.checked) {
			if (!edit_odd.value.match(/^\#[a-fA-F0-9]{6}$/) &&
		 	    !edit_odd.value.match(/^\$\w+$/)) {
		 		alert("Mauvaise valeur de couleur de fond-perdu recto.");
		 		return;
		 	}
			str += " odd=\"" + xmlEncode(edit_odd.value) + "\"";
		}
		if (check_even.checked) {
			if (!edit_even.value.match(/^\#[a-fA-F0-9]{6}$/) &&
		 	    !edit_even.value.match(/^\$\w+$/)) {
		 		alert("Mauvaise valeur de couleur de fond-perdu verso.");
		 		return;
		 	}
			str += " even=\"" + xmlEncode(edit_even.value) + "\"";
		}
		str += "/>\n";
	}
	str += "\t\t<cardsize";
	if (card_format == "standard") {
		str += " type=\"" + card_type + "\"";
		str += " landscape=\"" + card_landscape + "\"";
	} else {
		if (!edit_cardwidth.value.match(/^-?\d+$/) &&
		    !edit_cardwidth.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur de largeur de papier");
			return;
		}
		if (!edit_cardheight.value.match(/^-?\d+$/) &&
		    !edit_cardheight.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur de hauteur de papier");
			return;
		}
		str += " width=\"" + xmlEncode(edit_cardwidth.value) + unit_cardwidth + "\"";
		str += " height=\"" + xmlEncode(edit_cardheight.value) + unit_cardheight + "\"";
	}
	if (space_type == "same") {
		if (!edit_space.value.match(/^-?\d+$/) &&
		    !edit_space.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur d'espacement");
			return;
		}
		str += " space=\"" + edit_space.value + unit_space + "\"";
	} else if (space_type == "special") {
		if (!edit_spacewidth.value.match(/^-?\d+$/) &&
		    !edit_spacewidth.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur d'espacement horizontal");
			return;
		}
		if (!edit_spaceheight.value.match(/^-?\d+$/) &&
		    !edit_spaceheight.value.match(/^-?\d+\.\d+$/)) {
			alert("Mauvaise valeur d'espacement vertical");
			return;
		}
		str += " space-width=\"" + edit_spacewidth.value + unit_marginwidth + "\"";
		str += " space-height=\"" + edit_spaceheight.value + unit_marginheight + "\"";
	}
	str += "/>\n\t\t<!-- ajoutez ici les définitions de cartes -->\n\t\t";
	offset = str.length;
	str += "\n\t</deck>\n\
</pandocreon:carta-genius>";
	window.opener.carta.newFile();
	window.opener.carta.insert(str, offset);
	window.close();
}
