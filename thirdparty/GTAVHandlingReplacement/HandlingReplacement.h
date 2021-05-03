#pragma once

#ifdef HR_EXPORTS
#define HR_API extern "C" __declspec(dllexport)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define VER_MAJOR 1
#define VER_MINOR 0
#define VER_PATCH 0

#define HR_VER STR(VER_MAJOR) "."  STR(VER_MINOR) "." STR(VER_PATCH)
#else
#ifndef HR_RUNTIME
#define HR_API extern "C" __declspec(dllimport)
#else
// noop
#define HR_API
#endif
#endif

/**
 * \brief   Enables the replacement handling on the vehicle. Increments reference count if \p vehicle already has a
 *          replaced handling.
 *
 * \param [in] vehicle              Vehicle to enable the replacement handling on.
 * \param [out] ppHandlingData      Pointer to replaced handling pointer.\n
 *                                  nullptr when \p vehicle doesn't exist.
 * \return                          true    if this function call replaced the handling.\n
 *                                  false   if the handling already has been replaced, or\n
 *                                          if \p ppHandlingData is nullptr, or\n
 *                                          if \p vehicle doesn't exist.
 */
HR_API bool HR_Enable(int vehicle, void** ppHandlingData);

/**
 * \brief   Decrements reference count on \p vehicle. Disables the replacement handling on the vehicle when
 *          the reference count has reached 0.
 *
 * \param [in] vehicle              Vehicle to restore handling on.
 * \param [out] ppHandlingData      Pointer to current handling pointer after deletion.\n
 *                                  nullptr when \p vehicle doesn't exist.
 * \return                          true    if this function call restored the original handling.\n
 *                                  false   if the handling was not restored by this call, or\n
 *                                          if \p vehicle didn't have a replaced handling, or\n
 *                                          if \p ppHandlingData is nullptr, or\n
 *                                          if \p vehicle doesn't exist.
 */
HR_API bool HR_Disable(int vehicle, void** ppHandlingData);

/**
 * \brief   Get the current handling data
 *
 * \param [in] vehicle                      Vehicle to get handling from.
 * \param [out] ppHandlingDataOriginal      Pointer to the original handling.\n
 *                                          nullptr if \p vehicle doesn't exist.\n
 *                                          <b>DO NOT WRITE TO THIS!</b>
 * \param [out] ppHandlingDataReplaced      Pointer to the replaced handling.\n
 *                                          nullptr if \p vehicle doesn't exist or no replaced handling.
 * \return                                  true    if the handling has been replaced.\n
 *                                          false   if the handling is original, or\n
 *                                                  if either \p ppHandlingDataOriginal / \p ppHandlingDataReplaced
 *                                                  is null, or
 *                                                  if \p vehicle doesn't exist.\n
 */
HR_API bool HR_GetHandlingData(int vehicle, void** ppHandlingDataOriginal, void** ppHandlingDataReplaced);
