#pragma once

/*!
 * \file glheadless_api.h
 * \brief Declares API export macros.
 */


#ifdef _MSC_VER
#   define GLHEADLESS_API_EXPORT_DECLARATION __declspec(dllexport)
#   define GLHEADLESS_API_IMPORT_DECLARATION __declspec(dllimport)
#elif __GNUC__
#	define GLHEADLESS_API_EXPORT_DECLARATION __attribute__ ((visibility ("default")))
#	define GLHEADLESS_API_IMPORT_DECLARATION __attribute__ ((visibility ("default")))
#else
#   define GLHEADLESS_API_EXPORT_DECLARATION
#   define GLHEADLESS_API_IMPORT_DECLARATION
#endif 

#ifndef GLHEADLESS_STATIC
#ifdef GLHEADLESS_EXPORTS
#   define GLHEADLESS_API GLHEADLESS_API_EXPORT_DECLARATION
#else
#   define GLHEADLESS_API GLHEADLESS_API_IMPORT_DECLARATION
#endif
#else
#   define GLHEADLESS_API
#endif
