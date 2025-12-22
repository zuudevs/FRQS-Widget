### Modified
- renderer_d2d.hpp [Modified]
- resource_cache.hpp [Added]
- resource_cache.cpp [Added]
- event_types.hpp [Modified]
- widget.hpp [Modified]
- widget.cpp [Modified]
- text_input.cpp [Modified]

### Error
[{
	"resource": "/d:/Project/Fast Realibility Query System/FRQS Widget/include/widget/widget.hpp",
	"owner": "_generated_diagnostic_collection_name_#0",
	"code": "no_member",
	"severity": 8,
	"message": "No member named 'hitTest' in 'frqs::widget::IWidget'",
	"source": "clang",
	"startLineNumber": 33,
	"startColumn": 18,
	"endLineNumber": 33,
	"endColumn": 25,
	"origin": "extHost1"
},{
	"resource": "/d:/Project/Fast Realibility Query System/FRQS Widget/src/widget/text_input.cpp",
	"owner": "_generated_diagnostic_collection_name_#0",
	"code": "redefinition",
	"severity": 8,
	"message": "In included file: redefinition of 'FileDropEvent'",
	"source": "clang",
	"startLineNumber": 2,
	"startColumn": 10,
	"endLineNumber": 2,
	"endColumn": 33,
	"relatedInformation": [
		{
			"startLineNumber": 223,
			"startColumn": 8,
			"endLineNumber": 223,
			"endColumn": 21,
			"message": "Error occurred here",
			"resource": "/D:/Project/Fast Realibility Query System/FRQS Widget/include/event/event_types.hpp"
		},
		{
			"startLineNumber": 75,
			"startColumn": 8,
			"endLineNumber": 75,
			"endColumn": 21,
			"message": "Previous definition is here",
			"resource": "/D:/Project/Fast Realibility Query System/FRQS Widget/include/event/event.hpp"
		}
	],
	"origin": "extHost1"
},{
	"resource": "/d:/Project/Fast Realibility Query System/FRQS Widget/src/widget/text_input.cpp",
	"owner": "_generated_diagnostic_collection_name_#0",
	"code": {
		"value": "unused-includes",
		"target": {
			"$mid": 1,
			"path": "/guides/include-cleaner",
			"scheme": "https",
			"authority": "clangd.llvm.org"
		}
	},
	"severity": 4,
	"message": "Included header resource_cache.hpp is not used directly (fix available)",
	"source": "clangd",
	"startLineNumber": 3,
	"startColumn": 1,
	"endLineNumber": 3,
	"endColumn": 37,
	"tags": [
		1
	],
	"origin": "extHost1"
},{
	"resource": "/d:/Project/Fast Realibility Query System/FRQS Widget/src/widget/widget.cpp",
	"owner": "_generated_diagnostic_collection_name_#0",
	"code": "member_decl_does_not_match",
	"severity": 8,
	"message": "Out-of-line definition of 'hitTest' does not match any declaration in 'frqs::widget::Widget'",
	"source": "clang",
	"startLineNumber": 79,
	"startColumn": 18,
	"endLineNumber": 79,
	"endColumn": 25,
	"relatedInformation": [
		{
			"startLineNumber": 76,
			"startColumn": 7,
			"endLineNumber": 76,
			"endColumn": 13,
			"message": "Widget defined here",
			"resource": "/D:/Project/Fast Realibility Query System/FRQS Widget/include/widget/iwidget.hpp"
		}
	],
	"origin": "extHost1"
},{
	"resource": "/d:/Project/Fast Realibility Query System/FRQS Widget/src/widget/widget.cpp",
	"owner": "_generated_diagnostic_collection_name_#0",
	"code": "no_member",
	"severity": 8,
	"message": "No member named 'hitTest' in 'frqs::widget::IWidget'",
	"source": "clang",
	"startLineNumber": 92,
	"startColumn": 31,
	"endLineNumber": 92,
	"endColumn": 38,
	"origin": "extHost1"
},{
	"resource": "/d:/Project/Fast Realibility Query System/FRQS Widget/src/widget/widget.cpp",
	"owner": "_generated_diagnostic_collection_name_#0",
	"code": {
		"value": "unused-includes",
		"target": {
			"$mid": 1,
			"path": "/guides/include-cleaner",
			"scheme": "https",
			"authority": "clangd.llvm.org"
		}
	},
	"severity": 4,
	"message": "Included header window.hpp is not used directly (fixes available)",
	"source": "clangd",
	"startLineNumber": 3,
	"startColumn": 1,
	"endLineNumber": 3,
	"endColumn": 27,
	"tags": [
		1
	],
	"origin": "extHost1"
},{
	"resource": "/d:/Project/Fast Realibility Query System/FRQS Widget/src/widget/widget.cpp",
	"owner": "_generated_diagnostic_collection_name_#0",
	"code": {
		"value": "unused-includes",
		"target": {
			"$mid": 1,
			"path": "/guides/include-cleaner",
			"scheme": "https",
			"authority": "clangd.llvm.org"
		}
	},
	"severity": 4,
	"message": "Included header win32_safe.hpp is not used directly (fixes available)",
	"source": "clangd",
	"startLineNumber": 4,
	"startColumn": 1,
	"endLineNumber": 4,
	"endColumn": 35,
	"tags": [
		1
	],
	"origin": "extHost1"
},{
	"resource": "/d:/Project/Fast Realibility Query System/FRQS Widget/src/widget/widget.cpp",
	"owner": "_generated_diagnostic_collection_name_#0",
	"code": "-Wunused-parameter",
	"severity": 4,
	"message": "Unused parameter 'event'",
	"source": "clang",
	"startLineNumber": 106,
	"startColumn": 42,
	"endLineNumber": 106,
	"endColumn": 47,
	"tags": [
		1
	],
	"origin": "extHost1"
}]