/** 
 * @file llviewermenufile.h
 * @brief "File" menu in the main menu bar.
 *
 * $LicenseInfo:firstyear=2002&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef LLVIEWERMENUFILE_H
#define LLVIEWERMENUFILE_H

#include "llfoldertype.h"
#include "llassetstorage.h"
#include "llinventorytype.h"
#include "llfilepicker.h"
#include "llthread.h"
#include <queue>

#include "llviewerassetupload.h"
// <edit>
#include "llviewerinventory.h"

class NewResourceItemCallback : public LLInventoryCallback
{
 void fire(const LLUUID& inv_item);
};
// </edit>

class LLTransactionID;

const std::string BVHSTATUS[] =
{
    "E_ST_OK",
    "E_ST_EOF",
    "E_ST_NO_CONSTRAINT",
    "E_ST_NO_FILE",
    "E_ST_NO_HIER",
    "E_ST_NO_JOINT",
    "E_ST_NO_NAME",
    "E_ST_NO_OFFSET",
    "E_ST_NO_CHANNELS",
    "E_ST_NO_ROTATION",
    "E_ST_NO_AXIS",
    "E_ST_NO_MOTION",
    "E_ST_NO_FRAMES",
    "E_ST_NO_FRAME_TIME",
    "E_ST_NO_POS",
    "E_ST_NO_ROT",
    "E_ST_NO_XLT_FILE",
    "E_ST_NO_XLT_HEADER",
    "E_ST_NO_XLT_NAME",
    "E_ST_NO_XLT_IGNORE",
    "E_ST_NO_XLT_RELATIVE",
    "E_ST_NO_XLT_OUTNAME",
    "E_ST_NO_XLT_MATRIX",
    "E_ST_NO_XLT_MERGECHILD",
    "E_ST_NO_XLT_MERGEPARENT",
    "E_ST_NO_XLT_PRIORITY",
    "E_ST_NO_XLT_LOOP",
    "E_ST_NO_XLT_EASEIN",
    "E_ST_NO_XLT_EASEOUT",
    "E_ST_NO_XLT_HAND",
    "E_ST_NO_XLT_EMOTE",
    "E_ST_BAD_ROOT"
};

void init_menu_file();


void upload_new_resource(
    const std::string& src_filename,
    std::string name,
    std::string desc,
    S32 compression_info,
    LLFolderType::EType destination_folder_type,
    LLInventoryType::EType inv_type,
    U32 next_owner_perms,
    U32 group_perms,
    U32 everyone_perms,
    const std::string& display_name,
    LLAssetStorage::LLStoreAssetCallback callback,
    S32 expected_upload_cost,
    void *userdata);

bool upload_new_resource(
    LLResourceUploadInfo::ptr_t &uploadInfo,
    LLAssetStorage::LLStoreAssetCallback callback = nullptr,
    void *userdata = nullptr);

// The default callback functions, called when 'callback' == nullptr (for normal and temporary uploads).
// user_data must be a LLResourceData allocated with new (or nullptr).
void upload_done_callback(const LLUUID& uuid, void* user_data, S32 result, LLExtStat ext_status);
void temp_upload_callback(const LLUUID& uuid, void* user_data, S32 result, LLExtStat ext_status);

LLAssetID generate_asset_id_for_new_upload(const LLTransactionID& tid);

void assign_defaults_and_show_upload_message(
	LLAssetType::EType asset_type,
	LLInventoryType::EType& inventory_type,
	std::string& name,
	const std::string& display_name,
	std::string& description);

LLSD generate_new_resource_upload_capability_body(
	LLAssetType::EType asset_type,
	const std::string& name,
	const std::string& desc,
	LLFolderType::EType destination_folder_type,
	LLInventoryType::EType inv_type,
	U32 next_owner_perms,
	U32 group_perms,
	U32 everyone_perms);

class LLFilePickerThread : public LLThread
{	//multi-threaded file picker (runs system specific file picker in background and calls "notify" from main thread)
public:

	static std::queue<LLFilePickerThread*> sDeadQ;
	static LLMutex* sMutex;

	static void initClass();
	static void cleanupClass();
	static void clearDead();

	std::vector<std::string> mResponses;
	std::string mProposedName;

	LLFilePicker::ELoadFilter mLoadFilter;
	LLFilePicker::ESaveFilter mSaveFilter;
	bool mIsSaveDialog;
	bool mIsGetMultiple;

	LLFilePickerThread(LLFilePicker::ELoadFilter filter, bool get_multiple = false)
		: LLThread("file picker"), mLoadFilter(filter), mIsSaveDialog(false), mIsGetMultiple(get_multiple)
	{
	}

	LLFilePickerThread(LLFilePicker::ESaveFilter filter, const std::string &proposed_name)
		: LLThread("file picker"), mSaveFilter(filter), mIsSaveDialog(true), mProposedName(proposed_name)
	{
	}

	void getFile();

	void run() override;

	virtual void notify(const std::vector<std::string>& filenames) = 0;
};


class LLFilePickerReplyThread : public LLFilePickerThread
{
public:

	typedef boost::signals2::signal<void(const std::vector<std::string>& filenames, LLFilePicker::ELoadFilter load_filter, LLFilePicker::ESaveFilter save_filter)> file_picked_signal_t;

	LLFilePickerReplyThread(const file_picked_signal_t::slot_type& cb, LLFilePicker::ELoadFilter filter, bool get_multiple, const file_picked_signal_t::slot_type& failure_cb = file_picked_signal_t());
	LLFilePickerReplyThread(const file_picked_signal_t::slot_type& cb, LLFilePicker::ESaveFilter filter, const std::string &proposed_name, const file_picked_signal_t::slot_type& failure_cb = file_picked_signal_t());
	~LLFilePickerReplyThread();

	void notify(const std::vector<std::string>& filenames) override;

private:
	LLFilePicker::ELoadFilter	mLoadFilter;
	LLFilePicker::ESaveFilter	mSaveFilter;
	file_picked_signal_t*		mFilePickedSignal;
	file_picked_signal_t*		mFailureSignal;
};


#endif
