/**
 * @file llfloaterimsessiontab.cpp
 * @brief LLFloaterIMSessionTab class implements the common behavior of LNearbyChatBar
 * @brief and LLFloaterIMSession for hosting both in LLIMContainer
 *
 * $LicenseInfo:firstyear=2012&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2012, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llfloaterimsessiontab.h"

#include "llagent.h"
#include "llavataractions.h"
#include "llchatentry.h"
#include "llchathistory.h"
#include "llchiclet.h"
#include "llchicletbar.h"
#include "lldraghandle.h"
#include "llfloaterreg.h"
#include "llfloaterimsession.h"
#include "llfloaterimcontainer.h" // to replace separate IM Floaters with multifloater container
#include "lllayoutstack.h"
#include "lltoolbarview.h"
#include "llfloaterimnearbychat.h"

const F32 REFRESH_INTERVAL = 0.2f;

LLFloaterIMSessionTab::LLFloaterIMSessionTab(const LLSD& session_id)
  : LLTransientDockableFloater(NULL, true, session_id)
  ,  mIsP2PChat(false)
  ,  mExpandCollapseBtn(NULL)
  ,  mTearOffBtn(NULL)
  ,  mCloseBtn(NULL)
  ,  mSessionID(session_id.asUUID())
  , mConversationsRoot(NULL)
  , mScroller(NULL)
  , mChatHistory(NULL)
  , mInputEditor(NULL)
  , mInputEditorTopPad(0)
  , mRefreshTimer(new LLTimer())
  , mIsHostAttached(false)
{
	mSession = LLIMModel::getInstance()->findIMSession(mSessionID);

	mCommitCallbackRegistrar.add("IMSession.Menu.Action",
			boost::bind(&LLFloaterIMSessionTab::onIMSessionMenuItemClicked,  this, _2));
	mEnableCallbackRegistrar.add("IMSession.Menu.CompactExpandedModes.CheckItem",
			boost::bind(&LLFloaterIMSessionTab::onIMCompactExpandedMenuItemCheck, this, _2));
	mEnableCallbackRegistrar.add("IMSession.Menu.ShowModes.CheckItem",
			boost::bind(&LLFloaterIMSessionTab::onIMShowModesMenuItemCheck,   this, _2));
	mEnableCallbackRegistrar.add("IMSession.Menu.ShowModes.Enable",
			boost::bind(&LLFloaterIMSessionTab::onIMShowModesMenuItemEnable,  this, _2));
}

LLFloaterIMSessionTab::~LLFloaterIMSessionTab()
{
	delete mRefreshTimer;
}

//static
LLFloaterIMSessionTab* LLFloaterIMSessionTab::findConversation(const LLUUID& uuid)
{
	LLFloaterIMSessionTab* conv;

	if (uuid.isNull())
	{
		conv = LLFloaterReg::findTypedInstance<LLFloaterIMSessionTab>("nearby_chat");
	}
	else
	{
		conv = LLFloaterReg::findTypedInstance<LLFloaterIMSessionTab>("impanel", LLSD(uuid));
	}

	return conv;
};

//static
LLFloaterIMSessionTab* LLFloaterIMSessionTab::getConversation(const LLUUID& uuid)
{
	LLFloaterIMSessionTab* conv;

	if (uuid.isNull())
	{
		conv = LLFloaterReg::getTypedInstance<LLFloaterIMSessionTab>("nearby_chat");
	}
	else
	{
		conv = LLFloaterReg::getTypedInstance<LLFloaterIMSessionTab>("impanel", LLSD(uuid));
	}

	return conv;
};

void LLFloaterIMSessionTab::setVisible(BOOL visible)
{
	LLTransientDockableFloater::setVisible(visible);

	if(visible)
	{
			LLFloaterIMSessionTab::addToHost(mSessionID);
	}
    setFocus(visible);
}

/*virtual*/
void LLFloaterIMSessionTab::setFocus(BOOL focus)
{
	LLTransientDockableFloater::setFocus(focus);

    //Redirect focus to input editor
    if (focus)
	{
    	updateMessages();

        if (mInputEditor)
        {
    	    mInputEditor->setFocus(TRUE);
        }
	}
}


void LLFloaterIMSessionTab::addToHost(const LLUUID& session_id)
{
	if ((session_id.notNull() && !gIMMgr->hasSession(session_id))
			|| !LLFloaterIMSessionTab::isChatMultiTab())
	{
		return;
	}

	// Get the floater: this will create the instance if it didn't exist
	LLFloaterIMSessionTab* conversp = LLFloaterIMSessionTab::getConversation(session_id);
	if (conversp)
	{
		LLFloaterIMContainer* floater_container = LLFloaterIMContainer::getInstance();

		// Do not add again existing floaters
		if (floater_container && !conversp->isHostAttached())
		{
			conversp->setHostAttached(true);

			if (!conversp->isNearbyChat()
					|| gSavedSettings.getBOOL("NearbyChatIsNotTornOff"))
			{
				floater_container->addFloater(conversp, FALSE, LLTabContainer::END);

				if (!floater_container->getVisible())
				{
					LLFloaterReg::toggleInstanceOrBringToFront("im_container");			
				}
			}
			else
			{
				// setting of the "potential" host for Nearby Chat: this sequence sets
				// LLFloater::mHostHandle = NULL (a current host), but
				// LLFloater::mLastHostHandle = floater_container (a "future" host)
				conversp->setHost(floater_container);
				conversp->setHost(NULL);
			}
			// Added floaters share some state (like sort order) with their host
			conversp->setSortOrder(floater_container->getSortOrder());
		}
	}
}

BOOL LLFloaterIMSessionTab::postBuild()
{
	BOOL result;

	mCloseBtn = getChild<LLButton>("close_btn");
	mCloseBtn->setCommitCallback(boost::bind(&LLFloater::onClickClose, this));

	mExpandCollapseBtn = getChild<LLButton>("expand_collapse_btn");
	mExpandCollapseBtn->setClickedCallback(boost::bind(&LLFloaterIMSessionTab::onSlide, this));

	mTearOffBtn = getChild<LLButton>("tear_off_btn");
	mTearOffBtn->setCommitCallback(boost::bind(&LLFloaterIMSessionTab::onTearOffClicked, this));

	mParticipantListPanel = getChild<LLLayoutPanel>("speakers_list_panel");
	
	// Add a scroller for the folder (participant) view
	LLRect scroller_view_rect = mParticipantListPanel->getRect();
	scroller_view_rect.translate(-scroller_view_rect.mLeft, -scroller_view_rect.mBottom);
	LLScrollContainer::Params scroller_params(LLUICtrlFactory::getDefaultParams<LLFolderViewScrollContainer>());
	scroller_params.rect(scroller_view_rect);
	mScroller = LLUICtrlFactory::create<LLFolderViewScrollContainer>(scroller_params);
	mScroller->setFollowsAll();
	
	// Insert that scroller into the panel widgets hierarchy
	mParticipantListPanel->addChild(mScroller);	
	
	mChatHistory = getChild<LLChatHistory>("chat_history");

	mInputEditor = getChild<LLChatEntry>("chat_editor");
	mInputEditor->setTextExpandedCallback(boost::bind(&LLFloaterIMSessionTab::reshapeChatHistory, this));
	mInputEditor->setCommitOnFocusLost( FALSE );
	mInputEditor->setPassDelete(TRUE);
	mInputEditor->setFont(LLViewerChat::getChatFont());

	mInputEditorTopPad = mChatHistory->getRect().mBottom - mInputEditor->getRect().mTop;

	setOpenPositioning(LLFloaterEnums::POSITIONING_RELATIVE);

	mSaveRect = isTornOff();
	initRectControl();

	if (isChatMultiTab())
	{
		result = LLFloater::postBuild();
	}
	else
	{
		result = LLDockableFloater::postBuild();
	}

	// Now ready to build the conversation and participants list
	buildConversationViewParticipant();
	refreshConversation();
		
	// Zero expiry time is set only once to allow initial update.
	mRefreshTimer->setTimerExpirySec(0);
	mRefreshTimer->start();

	return result;
}

LLParticipantList* LLFloaterIMSessionTab::getParticipantList()
{
	return dynamic_cast<LLParticipantList*>(LLFloaterIMContainer::getInstance()->getSessionModel(mSessionID));
}

void LLFloaterIMSessionTab::draw()
{
	if (mRefreshTimer->hasExpired())
	{
		LLParticipantList* item = getParticipantList();
		if (item)
		{
			// Update all model items
			item->update();
			// If the model and view list diverge in count, rebuild
			// Note: this happens sometimes right around init (add participant events fire but get dropped) and is the cause
			// of missing participants, often, the user agent itself. As there will be no other event fired, there's
			// no other choice but get those inconsistencies regularly (and lightly) checked and scrubbed.
			if (item->getChildrenCount() != mConversationsWidgets.size())
			{
				buildConversationViewParticipant();
			}
			refreshConversation();
		}

		// Restart the refresh timer
		mRefreshTimer->setTimerExpirySec(REFRESH_INTERVAL);
	}
	
	LLTransientDockableFloater::draw();
}

void LLFloaterIMSessionTab::enableDisableCallBtn()
{
    getChildView("voice_call_btn")->setEnabled(
    		mSessionID.notNull()
    		&& mSession
    		&& mSession->mSessionInitialized
    		&& LLVoiceClient::getInstance()->voiceEnabled()
    		&& LLVoiceClient::getInstance()->isVoiceWorking()
    		&& mSession->mCallBackEnabled);
}

void LLFloaterIMSessionTab::onFocusReceived()
{
	setBackgroundOpaque(true);

	if (mSessionID.notNull() && isInVisibleChain())
	{
		LLIMModel::instance().sendNoUnreadMessages(mSessionID);
	}

	LLTransientDockableFloater::onFocusReceived();

	LLFloaterIMContainer* container = LLFloaterReg::getTypedInstance<LLFloaterIMContainer>("im_container");
	if (container)
	{
		container->selectConversationPair(mSessionID, true);
		container->showStub(! getHost());
	}
}

void LLFloaterIMSessionTab::onFocusLost()
{
	setBackgroundOpaque(false);
	LLTransientDockableFloater::onFocusLost();
}

std::string LLFloaterIMSessionTab::appendTime()
{
	time_t utc_time;
	utc_time = time_corrected();
	std::string timeStr ="["+ LLTrans::getString("TimeHour")+"]:["
		+LLTrans::getString("TimeMin")+"]";

	LLSD substitution;

	substitution["datetime"] = (S32) utc_time;
	LLStringUtil::format (timeStr, substitution);

	return timeStr;
}

void LLFloaterIMSessionTab::appendMessage(const LLChat& chat, const LLSD &args)
{

	// Update the participant activity time
	LLFloaterIMContainer* im_box = LLFloaterIMContainer::findInstance();
	if (im_box)
	{
		im_box->setTimeNow(mSessionID,chat.mFromID);
	}
	

	LLChat& tmp_chat = const_cast<LLChat&>(chat);

	if(tmp_chat.mTimeStr.empty())
		tmp_chat.mTimeStr = appendTime();

	if (!chat.mMuted)
	{
		tmp_chat.mFromName = chat.mFromName;
		LLSD chat_args;
		if (args) chat_args = args;
		chat_args["use_plain_text_chat_history"] =
				gSavedSettings.getBOOL("PlainTextChatHistory");
		chat_args["show_time"] = gSavedSettings.getBOOL("IMShowTime");
		chat_args["show_names_for_p2p_conv"] =
				!mIsP2PChat || gSavedSettings.getBOOL("IMShowNamesForP2PConv");

		if (mChatHistory)
		{
			mChatHistory->appendMessage(chat, chat_args);
		}
	}
}


void LLFloaterIMSessionTab::buildConversationViewParticipant()
{
	// Clear the widget list since we are rebuilding afresh from the model
	conversations_widgets_map::iterator widget_it = mConversationsWidgets.begin();
	while (widget_it != mConversationsWidgets.end())
	{
		removeConversationViewParticipant(widget_it->first);
		// Iterators are invalidated by erase so we need to pick begin again
		widget_it = mConversationsWidgets.begin();
	}
	
	// Get the model list
	LLParticipantList* item = getParticipantList();
	if (!item)
	{
		// Nothing to do if the model list is inexistent
		return;
	}
	
	// Create or recreate the root folder: this is a dummy folder (not shown) but required by the LLFolderView architecture 
	// We need to redo this when rebuilding as the session id (mSessionID) *may* have changed
	if (mConversationsRoot)
	{
		// Remove the old root if any
		mScroller->removeChild(mConversationsRoot);
	}
	// Create the root using an ad-hoc base item
	LLConversationItem* base_item = new LLConversationItem(mSessionID, mConversationViewModel);
    LLFolderView::Params p(LLUICtrlFactory::getDefaultParams<LLFolderView>());
    p.rect = LLRect(0, 0, getRect().getWidth(), 0);
    p.parent_panel = mParticipantListPanel;
    p.listener = base_item;
    p.view_model = &mConversationViewModel;
    p.root = NULL;
    p.use_ellipses = true;
	mConversationsRoot = LLUICtrlFactory::create<LLFolderView>(p);
    mConversationsRoot->setCallbackRegistrar(&mCommitCallbackRegistrar);
	// Attach that root to the scroller
	mScroller->addChild(mConversationsRoot);
	mConversationsRoot->setScrollContainer(mScroller);
	mConversationsRoot->setFollowsAll();
	mConversationsRoot->addChild(mConversationsRoot->mStatusTextBox);
	
	// Create the participants widgets now
	LLFolderViewModelItemCommon::child_list_t::const_iterator current_participant_model = item->getChildrenBegin();
	LLFolderViewModelItemCommon::child_list_t::const_iterator end_participant_model = item->getChildrenEnd();
	while (current_participant_model != end_participant_model)
	{
		LLConversationItem* participant_model = dynamic_cast<LLConversationItem*>(*current_participant_model);
		addConversationViewParticipant(participant_model);
		current_participant_model++;
	}
}

void LLFloaterIMSessionTab::addConversationViewParticipant(LLConversationItem* participant_model)
{
	// Check if the model already has an associated view
	LLUUID uuid = participant_model->getUUID();
	LLFolderViewItem* widget = get_ptr_in_map(mConversationsWidgets,uuid);
	
	// If not already present, create the participant view and attach it to the root, otherwise, just refresh it
	if (widget)
	{
		updateConversationViewParticipant(uuid); // overkill?
	}
	else
	{
		LLConversationViewParticipant* participant_view = createConversationViewParticipant(participant_model);
		mConversationsWidgets[uuid] = participant_view;
		participant_view->addToFolder(mConversationsRoot);
		participant_view->addToSession(mSessionID);
		participant_view->setVisible(TRUE);
	}
}

void LLFloaterIMSessionTab::removeConversationViewParticipant(const LLUUID& participant_id)
{
	LLFolderViewItem* widget = get_ptr_in_map(mConversationsWidgets,participant_id);
	if (widget)
	{
		mConversationsRoot->extractItem(widget);
		delete widget;
		mConversationsWidgets.erase(participant_id);
	}
}

void LLFloaterIMSessionTab::updateConversationViewParticipant(const LLUUID& participant_id)
{
	LLFolderViewItem* widget = get_ptr_in_map(mConversationsWidgets,participant_id);
	if (widget)
	{
		widget->refresh();
	}
}

void LLFloaterIMSessionTab::refreshConversation()
{
	// Note: We collect participants names to change the session name only in the case of ad-hoc conversations
	bool is_ad_hoc = (mSession ? mSession->isAdHocSessionType() : false);
	uuid_vec_t participants_uuids; // uuids vector for building the added participants name string
	// For P2P chat, we still need to update the session name who may have changed (switch display name for instance)
	if (mIsP2PChat && mSession)
	{
		participants_uuids.push_back(mSession->mOtherParticipantID);
	}

	conversations_widgets_map::iterator widget_it = mConversationsWidgets.begin();
	while (widget_it != mConversationsWidgets.end())
	{
		// Add the participant to the list except if it's the agent itself (redundant)
		if (is_ad_hoc && (widget_it->first != gAgentID))
		{
			participants_uuids.push_back(widget_it->first);
		}
		widget_it->second->refresh();
		widget_it->second->setVisible(TRUE);
		++widget_it;
	}
	if (is_ad_hoc || mIsP2PChat)
	{
		// Build the session name and update it
		std::string session_name;
		if (participants_uuids.size() != 0)
		{
			LLAvatarActions::buildResidentsString(participants_uuids, session_name);
		}
		else
		{
			session_name = LLIMModel::instance().getName(mSessionID);
		}
		updateSessionName(session_name);
	}
	
	mConversationViewModel.requestSortAll();
	if(mConversationsRoot != NULL)
	{
		mConversationsRoot->arrangeAll();
		mConversationsRoot->update();
	}
	updateHeaderAndToolbar();
	refresh();
}

// Copied from LLFloaterIMContainer::createConversationViewParticipant(). Refactor opportunity!
LLConversationViewParticipant* LLFloaterIMSessionTab::createConversationViewParticipant(LLConversationItem* item)
{
    LLRect panel_rect = mParticipantListPanel->getRect();
	
	LLConversationViewParticipant::Params params;
	params.name = item->getDisplayName();
	params.root = mConversationsRoot;
	params.listener = item;
	params.rect = LLRect (0, 24, panel_rect.getWidth(), 0); // *TODO: use conversation_view_participant.xml itemHeight value in lieu of 24
	params.tool_tip = params.name;
	params.participant_id = item->getUUID();
	
	return LLUICtrlFactory::create<LLConversationViewParticipant>(params);
}

void LLFloaterIMSessionTab::setSortOrder(const LLConversationSort& order)
{
	mConversationViewModel.setSorter(order);
	mConversationsRoot->arrangeAll();
	refreshConversation();
}

void LLFloaterIMSessionTab::onIMSessionMenuItemClicked(const LLSD& userdata)
{
	std::string item = userdata.asString();

	if (item == "compact_view" || item == "expanded_view")
	{
		gSavedSettings.setBOOL("PlainTextChatHistory", item == "compact_view");
	}
	else
	{
		bool prev_value = gSavedSettings.getBOOL(item);
		gSavedSettings.setBOOL(item, !prev_value);
	}

	LLFloaterIMSessionTab::processChatHistoryStyleUpdate();
}


bool LLFloaterIMSessionTab::onIMCompactExpandedMenuItemCheck(const LLSD& userdata)
{
	std::string item = userdata.asString();
	bool is_plain_text_mode = gSavedSettings.getBOOL("PlainTextChatHistory");

	return is_plain_text_mode? item == "compact_view" : item == "expanded_view";
}


bool LLFloaterIMSessionTab::onIMShowModesMenuItemCheck(const LLSD& userdata)
{
	return gSavedSettings.getBOOL(userdata.asString());
}

// enable/disable states for the "show time" and "show names" items of the show-modes menu
bool LLFloaterIMSessionTab::onIMShowModesMenuItemEnable(const LLSD& userdata)
{
	std::string item = userdata.asString();
	bool plain_text = gSavedSettings.getBOOL("PlainTextChatHistory");
	bool is_not_names = (item != "IMShowNamesForP2PConv");
	return (plain_text && (is_not_names || mIsP2PChat));
}

void LLFloaterIMSessionTab::hideOrShowTitle()
{
	const LLFloater::Params& default_params = LLFloater::getDefaultParams();
	S32 floater_header_size = default_params.header_height;
	LLView* floater_contents = getChild<LLView>("contents_view");

	LLRect floater_rect = getLocalRect();
	S32 top_border_of_contents = floater_rect.mTop - (isTornOff()? floater_header_size : 0);
	LLRect handle_rect (0, floater_rect.mTop, floater_rect.mRight, top_border_of_contents);
	LLRect contents_rect (0, top_border_of_contents, floater_rect.mRight, floater_rect.mBottom);
	mDragHandle->setShape(handle_rect);
	mDragHandle->setVisible(isTornOff());
	floater_contents->setShape(contents_rect);
}

void LLFloaterIMSessionTab::updateSessionName(const std::string& name)
{
	mInputEditor->setLabel(LLTrans::getString("IM_to_label") + " " + name);
}

void LLFloaterIMSessionTab::hideAllStandardButtons()
{
	for (S32 i = 0; i < BUTTON_COUNT; i++)
	{
		if (mButtons[i])
		{
			// Hide the standard header buttons in a docked IM floater.
			mButtons[i]->setVisible(false);
		}
	}
}

void LLFloaterIMSessionTab::updateHeaderAndToolbar()
{
	// prevent start conversation before its container
    LLFloaterIMContainer::getInstance();

	bool is_torn_off = checkIfTornOff();
	if (!is_torn_off)
	{
		hideAllStandardButtons();
	}

	hideOrShowTitle();

	// Participant list should be visible only in torn off floaters.
	bool is_participant_list_visible =
			is_torn_off
			&& gSavedSettings.getBOOL("IMShowControlPanel")
			&& !mIsP2PChat;

	mParticipantListPanel->setVisible(is_participant_list_visible);

	// Display collapse image (<<) if the floater is hosted
	// or if it is torn off but has an open control panel.
	bool is_expanded = !is_torn_off || is_participant_list_visible;
	mExpandCollapseBtn->setImageOverlay(getString(is_expanded ? "collapse_icon" : "expand_icon"));

	// toggle floater's drag handle and title visibility
	if (mDragHandle)
	{
		mDragHandle->setTitleVisible(is_torn_off);
	}

	// The button (>>) should be disabled for torn off P2P conversations.
	mExpandCollapseBtn->setEnabled(!is_torn_off || !mIsP2PChat);

	mTearOffBtn->setImageOverlay(getString(is_torn_off? "return_icon" : "tear_off_icon"));
	mTearOffBtn->setToolTip(getString(!is_torn_off? "tooltip_to_separate_window" : "tooltip_to_main_window"));

	mCloseBtn->setVisible(!is_torn_off && !mIsNearbyChat);

	enableDisableCallBtn();

	showTranslationCheckbox();
}

void LLFloaterIMSessionTab::reshapeChatHistory()
{
	LLRect chat_rect  = mChatHistory->getRect();
	LLRect input_rect = mInputEditor->getRect();

	int delta_height = chat_rect.mBottom - (input_rect.mTop + mInputEditorTopPad);

	chat_rect.setLeftTopAndSize(chat_rect.mLeft, chat_rect.mTop, chat_rect.getWidth(), chat_rect.getHeight() + delta_height);
	mChatHistory->setShape(chat_rect);
}

void LLFloaterIMSessionTab::showTranslationCheckbox(BOOL show)
{
	getChild<LLUICtrl>("translate_chat_checkbox_lp")->setVisible(mIsNearbyChat? show : FALSE);
}

// static
void LLFloaterIMSessionTab::processChatHistoryStyleUpdate()
{
	LLFloaterReg::const_instance_list_t& inst_list = LLFloaterReg::getFloaterList("impanel");
	for (LLFloaterReg::const_instance_list_t::const_iterator iter = inst_list.begin();
			iter != inst_list.end(); ++iter)
	{
		LLFloaterIMSession* floater = dynamic_cast<LLFloaterIMSession*>(*iter);
		if (floater)
		{
			floater->reloadMessages();
		}
	}

	LLFloaterIMNearbyChat* nearby_chat = LLFloaterReg::findTypedInstance<LLFloaterIMNearbyChat>("nearby_chat");
	if (nearby_chat)
	{
             nearby_chat->reloadMessages();
	}
}

void LLFloaterIMSessionTab::updateCallBtnState(bool callIsActive)
{
	getChild<LLButton>("voice_call_btn")->setImageOverlay(
			callIsActive? getString("call_btn_stop") : getString("call_btn_start"));
    enableDisableCallBtn();

}

void LLFloaterIMSessionTab::onSlide(LLFloaterIMSessionTab* self)
{
	LLFloaterIMContainer* host_floater = dynamic_cast<LLFloaterIMContainer*>(self->getHost());
	if (host_floater)
	{
		// Hide the messages pane if a floater is hosted in the Conversations
		host_floater->collapseMessagesPane(true);
	}
	else ///< floater is torn off
	{
		if (!self->mIsP2PChat)
		{
			bool expand = !self->mParticipantListPanel->getVisible();

			// Expand/collapse the IM control panel
			self->mParticipantListPanel->setVisible(expand);

			gSavedSettings.setBOOL("IMShowControlPanel", expand);

			self->mExpandCollapseBtn->setImageOverlay(self->getString(expand ? "collapse_icon" : "expand_icon"));
		}
	}
}

/*virtual*/
void LLFloaterIMSessionTab::onOpen(const LLSD& key)
{
	if (!checkIfTornOff())
	{
		LLFloaterIMContainer* host_floater = dynamic_cast<LLFloaterIMContainer*>(getHost());
		// Show the messages pane when opening a floater hosted in the Conversations
		host_floater->collapseMessagesPane(false);
	}
}

// virtual
void LLFloaterIMSessionTab::onClose(bool app_quitting)
{
	// Always suppress the IM from the conversations list on close if present for any reason
	if (LLFloaterIMSessionTab::isChatMultiTab())
	{
		LLFloaterIMContainer* im_box = LLFloaterIMContainer::findInstance();
		if (im_box)
		{
            im_box->removeConversationListItem(mKey);
        }
    }
}

void LLFloaterIMSessionTab::onTearOffClicked()
{
    setFollows(isTornOff()? FOLLOWS_ALL : FOLLOWS_NONE);
    mSaveRect = isTornOff();
    initRectControl();
	LLFloater::onClickTearOff(this);
	refreshConversation();
}

// static
bool LLFloaterIMSessionTab::isChatMultiTab()
{
	// Restart is required in order to change chat window type.
	return true;
}

bool LLFloaterIMSessionTab::checkIfTornOff()
{
	bool isTorn = !getHost();
	
	if (isTorn != isTornOff())
	{
		setTornOff(isTorn);
		refreshConversation();
	}

	return isTorn;
}
