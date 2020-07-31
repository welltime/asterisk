# res_json
[![GitHub license](https://img.shields.io/github/license/ernaniaz/asterisk-res_json.svg)](https://github.com/ernaniaz/asterisk-res_json)

__res_json__ is a wrapper module around a JSON library, that gives you the capability to handle JSON documents inside an asterisk dialplan. If I already have you confused with terms like "asterisk dialplan" (http://www.asterisk.org) or "JSON" (http://www.json.org/), I will not insist in getting you interested for now. But, if you ever had to use external agi scripts just to obtain the value of a variable from a rest API, read on - this may help you. And yeah, I will try to make it easy to understand.

This version of __res_json__ has been updated to work with Asterisk 16, fixed some documentations and looking to make the module more standard. There's also a patch available to apply easily to Asterisk.


## Basic functions

A lot of API's these days send back responses in a JSON format. Calling the API and obtaining the response is the "easy" part, since other people worked hard to create asterisk's func_curl (whic is a wrapper around the curl library). Now we're going to deal with the other part - parsing the JSON HTTP response, and obtaining the values for the variables we need.

Here's the basic idea:

```
exten => s,n,set(json=${CURL(http://api.dataprovider.com/somefunction?param=value)})
exten => s,n,set(myvariable=${JSONELEMENT(json,path/to/element)})
```

This is going to call an API that returns a JSON document, and set the value of an asterisk dialplan variable to something found inside that JSON document. But this is not all: you can use the apps and functions inside __res_json__ to modify, or even construct from scratch, JSON documents. You can then post them, again using curl, to other API's:

```
exten => s,n,JSONSet(json,path/to/element,${newvalue})
exten => s,n,JSONAdd(json,path/to,number,newelement,5)
exten => s,n,set(response=${CURL(http://api.datareceiver.com/somefunction,${json})})
```

More detailed info below.


## Install

__res_json__ needs to be built into asterisk. I'm working with the people that take care of the asterisk distribution so we can include this module in the main distribution. Until that happens, you will need to compile asterisk from source and have it take care of the JSON library. Therefore, step by step, this is what you have to do.

(1) Obtain the asterisk source code, from https://www.asterisk.org/downloads. Unzip and untar it, but dont proceed to building it yet.

(2) Cd into the directory where you unzipped / untarred asterisk (the "asterisk root"), and get the __res_json__ module (git must be installed on your machine): `git clone git://github.com/drivefast/asterisk-res_json.git`

(3) We now need to move the source files to their appropriate places in the asterisk directory. A shell script was provided for that, so run `./asterisk-res_json/install.sh`. After it runs, you need to manually edit `addons/Makefile` (sorry about that, but I really don't have a better solution):
- Add `res_json` to the `ALL_C_MODS` macro
- Explicitely tell the linker to add the res_json symbols by adding a line like

`res_json.so: cJSON.o res_json.o`

(4) Edit the file `main/asterisk.exports.in` and add the following line next to the similar ones:

`LINKER_SYMBOL_PREFIXcJSON_*;`

(5) Only now proceed with building asterisk (`./configure; make menuconfig; make; make install`).  if you already built asterisk from source in this directory, you may need to run `./bootstrap.sh` before running `./configure`.

(6) Start asterisk, login to its console, and try `core show function JSONELEMENT`. You should get an usage description.


## Alternative install

There's also a patch available created to Asterisk 16.6.1. You can just use at your Asterisk 16.6.1 source directory:

```bash
patch -p1 < asterisk16-res_json.patch
```

After that, just configure, compile and install.


## What'd you get

A bunch of apps and functions:

- `JSONELEMENT(doc,path)` (r/o function) - Gets the value of an element at a given path in a JSON document
- `JSONVariables(doc)` (app) - Reads a single level JSON document (dictionary) into dialplan variables
- `JSONAdd(doc,path,elemtype,name,value)` (app) - Adds an element to the JSON document at the given path
- `JSONSet(doc,path,newvalue)` (app) - Changes the value of an element in the JSON document
- `JSONDelete(doc,path)` (app) - Deletes an element in the JSON document
- `JSONPRETTY(doc)` (r/o function) - Formats a JSON document for nice printing
- `JSONCOMPRESS(doc)` (r/o function) - Formats a JSON document for minimum footprint

None of the functions or the apps above would fail in such a way that would terminate the call. If any of them would need to return an abnormal result, they would do so by setting the value of a dialplan variable called `JSONRESULT`. These values are:

* `ASTJSON_OK` (0) - The operation was successful

* `ASTJSON_UNDECIDED` (1) - The operation was aborted mid-way and the results are not guaranteed

* `ASTJSON_ARG_NEEDED` (2) - Missing or invalid argument type

* `ASTJSON_PARSE_ERROR` (3) - The string that was supposed to be a JSON document could not be parsed

* `ASTJSON_NOTFOUND` (4) - The expected element could not be found at the given path

* `ASTJSON_INVALID_TYPE` (5) - Invalid element type for a JSONAdd or JSONSet operation

* `ASTJSON_ADD_FAILED` (6) - The JSONAdd operation failed

* `ASTJSON_SET_FAILED` (7) - The JSONSet operation failed

* `ASTJSON_DELETE_FAILED` (8) - The JSONDelete operation failed

__IMPORTANT NOTE:__ All the functions and apps expect **the name** of a dialplan variable containing the JSON document, instead of the parseable string itself. For example, if the document is stored in the variable named `json`, we would call the function and execute an app using `json` as parameter:

```
exten => s,n,set(el=${JSONELEMENT(json,path/to/elem)})
exten => s,n,JSONSet(json,path/to/elem,123)
```

And __!!NOT!!__ the contents of the JSON variable, like in:

```
exten => s,n,set(el=${JSONELEMENT(${json},path/to/elem)})  ;; WRONG
exten => s,n,JSONSet(${json},path/to/elem,123)             ;; WRONG
```

The decision on this type of usage was made because, typically, the JSON representations contain a lot of commas. Escaping the JSON content such that the arguments are parsed correctly becomes therefore pretty complicated.


## Apps and functions

- `JSONELEMENT(doc,path)`

>Returns the value of an element at the given path. The element type is set in the dialplan variable `JSONTYPE`. Depending on the type of the JSON variable, the values are:
>
>   True, False => Returned values are `1` or `0` respectively; `JSONTYPE` is `bool`
>
>   NULL => Returned value is an empty string; `JSONTYPE` is `null`
>
>   Number => Returned value is a number; `JSONTYPE` is `number`
>
>   String => Returned value is a number; `JSONTYPE` is `string`
>
>   Array => Returned value is a JSON representation of an array; `JSONTYPE` is `array`
>
>   Object => Returned value is a JSON representation of the underlying object; `JSONTYPE` is `node`
>
>Parameters:
>
>   _doc_: The name (not the contents!) of a variable that contains the JSON document
>
>   _path_: Path to the element we're looking for (like `/path/to/element`, or `/path/to/element/3` to identify the element with index 3 in an array)


- `JSONVariables(doc)`

>Reads a single level JSON document into dialplan variables with the same names. The JSON document is considered to be the representation of a dictionary, or key-value pairs, containing scalar values. For example `{"one":1,"two":"deuce","three":"III"}` will set the values of the dialplan variables `one`, `two` and `three` to the values `1`, `deuce`, and `III` respectively. Depending on the type of each variable, their possible values are:
>
>   True, False => `1`, `0`
>
>   NULL => Resulting asterisk variable will contain an empty string
>
>   Number, String => The number or the string
>
>   Array => The string `!array!` (array values cannot be returned into a single variable)
>
>   Object => String, the JSON representation of the underlying object parameters
>
>Parameters:
>
>   _doc_: The name (not the contents!) of a variable that contains the JSON document


- `JSONAdd(doc,path,elemtype,[name][,value])`

>Adds an element to the JSON document at the given path. The value of the variable that contains the JSON document is updated to reflect the change. The element to be added has a type (_elemtype_), a _name_, and a _value_. _elemtype_ can be one of `bool`, `null`, `number`, `string`, `node` or `array`.  a `bool` "false" value is represented as either an empty string, `0`, `n`, `no`, `f` or `false` (case insensitive); any other value for a `bool` _elemtype_ is interpreted as true. For a `null` _elemtype_, the _value_ paramenter is ignored. The _value_ parameter is also ignored for an `array` _elemtype_: in this case, and an empty array is created. Further on, you may append elements to this array using repeated calls to the `JSONAdd` app. Something like this:

```
exten => s,n,JSONAdd(json,path/there,array,vec)
exten => s,n,JSONAdd(json,path/there/vec,string,,abcd)
exten => s,n,JSONAdd(json,path/there/vec,number,,1234)
exten => s,n,noop(${JSONELEMENT(json,path/there/vec/0)} & ${JSONELEMENT(json,path/there/vec/1)})
```

>The last line will display `abcd & 1234` to the console.
>
>Parameters:
>
>  _doc_: The name (not the contents!) of a variable that contains the JSON document
>
>  _path_: Path to the element to which we're adding (like `/path/to/element`, or `/path/to/element/3` to identify the element with index 3 in an array)
>
>  _elemtype_: Element type, one of `bool`, `null`, `number`, `string`, `node` or `array`
>
>  _name_: The name of the element to be added (may be missing if adding elements to an array)
>
>  _value_: Value to be set for the element we added


- `JSONSet(doc,path,newvalue)`

>Sets the value of the element in the JSON document at the given path. The value of the variable that contains the JSON document (_doc_) is updated to reflect the change. The element that changes the value preserves its name and its type, and must be a boolean, number, or string. The new value is converted to the type of the existing document. That means, if you would try to set the value of a number element to `abc`, its resulting value will be `0`, or if you try to set a boolean element to `13`, you will end up with it being `true`. To set a "false" value to a `bool` element, use an empty string, `0`, `n`, `no`, `f` or `false` (case insensitive); anything else is interpreted as `true`.
>
>Parameters:
>
>   _doc_: The name (not the contents!) of a variable that contains the JSON document
>
>   _path_: Path to the element to which we're adding (like `/path/to/element`, or `/path/to/element/3` to identify the element with index 3 in an array)
>
>   _newvalue_: Value to be set


- `JSONDelete(doc,path)`

>Delete the element at the given path, from the given document. The value of the variable that contains the JSON document (_doc_) is updated to reflect the change. You may delete any type of element.
>
>Parameters:
>
>   _doc_: The name (not the contents!) of a variable that contains the JSON document
>
>   _path_: Path to the element to which we're adding (like `/path/to/element`, or `/path/to/element/3` to identify the element with index 3 in an array)


- `JSONPRETTY(doc)`

>Returns the nicely formatted form of a JSON document, suitable for printing and easy reading. The function has cosmetic functionality only.
>
>Parameters:
>
>   _doc_: The name (not the contents!) of a variable that contains the JSON document. The value will not change.


- `JSONCOMPRESS(doc)`

>Returns the given JSON document formatted for a minimum footprint (eliminates all unnecessary characters). The function has cosmetic functionality only.
>
>Parameters:
>
>  _doc_: The name (not the contents!) of a variable that contains the JSON document. The value will not change.


## Author, licensing and credits

Radu Maierean
radu dot maierean at gmail

Copyright (c) 2010 Radu Maierean

The __res_json__ module is distributed under the GNU General Public License version 2. The GPL (version 2) is included in this source tree in the file COPYING.

The __res_json__ module is built on top of David Gamble's cJSON library. I used his code with very minor and insignificant changes, to make my picky compiler happy. The __res_json module__ is intended to be used with asterisk, so you will have to follow their usage and distribution policy.  and I guess so do I. I'm no lawyer and I have to take the safe route, and this is why I go with the same level of license restriction as asterisk does.
