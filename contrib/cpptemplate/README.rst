cpptempl
=================
This is a template engine for C++.

Copyright
==================
Author: Ryan Ginstrom
MIT License

Syntax
=================
Variables: {$variable_name}
Loops: {% for person in people %}Name: {$person.name}{% endfor %}
If: {% if person.name == "Bob" %}Full name: Robert{% endif %}


Usage
=======================
	wstring text = L"{% if item %}{$item}{% endif %}\n"
		L"{% if thing %}{$thing}{% endif %}" ;
	cpptempl::data_map data ;
	data[L"item"] = cpptempl::make_data(L"aaa") ;
	data[L"thing"] = cpptempl::make_data(L"bbb") ;

	wstring result = cpptempl::parse(text, data) ;

Handy Functions
========================
make_data() : Feed it a string, data_map, or data_list to create a data entry.
Example:
	data_map person ;
	person[L"name"] = make_data(L"Bob") ;
	person[L"occupation"] = make_data(L"Plumber") ;
	data_map data ;
	data[L"person"] = make_data(person) ;
	wstring result = parse(templ_text, data) ;