"""
Generic UI Locale Refresh System
Provides RefreshByMapping and RebuildDictionary for hot-reload support.
"""

import dbg

def RefreshByMapping(elementMap):
	"""
	Refresh UI elements using a mapping dictionary.

	Args:
		elementMap: Dict of {element_instance: "LOCALE_KEY_NAME"}

	Returns:
		Number of elements successfully refreshed
	"""
	import uiScriptLocale
	import localeInfo

	refreshCount = 0
	for element, localeKey in list(elementMap.items()):
		try:
			# Try uiScriptLocale first, then localeInfo
			if hasattr(uiScriptLocale, localeKey):
				text = getattr(uiScriptLocale, localeKey)
			elif hasattr(localeInfo, localeKey):
				text = getattr(localeInfo, localeKey)
			else:
				dbg.TraceError("LocaleRefresh: key not found: %s" % localeKey)
				continue

			if hasattr(element, 'SetText'):
				element.SetText(text)
				refreshCount += 1
		except Exception as e:
			dbg.TraceError("LocaleRefresh: failed for key %s: %s" % (localeKey, str(e)))

	return refreshCount

def RebuildDictionary(template, localeModule="localeInfo"):
	"""
	Rebuild a dictionary with fresh locale strings.

	Args:
		template: Dict of {key: "LOCALE_CONSTANT_NAME"}
		localeModule: "localeInfo" or "uiScriptLocale"

	Returns:
		New dictionary with resolved locale values
	"""
	import localeInfo
	import uiScriptLocale

	module = localeInfo if localeModule == "localeInfo" else uiScriptLocale
	newDict = {}

	for key, localeKey in list(template.items()):
		if hasattr(module, localeKey):
			newDict[key] = getattr(module, localeKey)
		else:
			dbg.TraceError("LocaleRefresh: key not found: %s" % localeKey)

	return newDict
