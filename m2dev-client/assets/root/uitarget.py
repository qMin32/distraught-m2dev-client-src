import app
import ui
import player
import net
import wndMgr
import messenger
import guild
import chr
import nonplayer
import localeInfo
import constInfo
import uiCommon

class TargetBoard(ui.ThinBoard):

	# Locale-independent button identifiers (never use translated strings as dict keys)
	BTN_WHISPER = 0
	BTN_EXCHANGE = 1
	BTN_FIGHT = 2
	BTN_ACCEPT_FIGHT = 3
	BTN_AVENGE = 4
	BTN_FRIEND = 5
	BTN_INVITE_PARTY = 6
	BTN_LEAVE_PARTY = 7
	BTN_EXCLUDE = 8
	BTN_INVITE_GUILD = 9
	BTN_REMOVE_GUILD = 10
	BTN_DISMOUNT = 11
	BTN_EXIT_OBSERVER = 12
	BTN_VIEW_EQUIPMENT = 13
	BTN_REQUEST_ENTER_PARTY = 14
	BTN_BUILDING_DESTROY = 15
	BTN_EMOTION_ALLOW = 16
	BTN_VOTE_BLOCK_CHAT = 17

	# (buttonId, localeInfo attribute name) — text resolved at __init__ time
	BUTTON_CONFIG = (
		(BTN_WHISPER, "TARGET_BUTTON_WHISPER"),
		(BTN_EXCHANGE, "TARGET_BUTTON_EXCHANGE"),
		(BTN_FIGHT, "TARGET_BUTTON_FIGHT"),
		(BTN_ACCEPT_FIGHT, "TARGET_BUTTON_ACCEPT_FIGHT"),
		(BTN_AVENGE, "TARGET_BUTTON_AVENGE"),
		(BTN_FRIEND, "TARGET_BUTTON_FRIEND"),
		(BTN_INVITE_PARTY, "TARGET_BUTTON_INVITE_PARTY"),
		(BTN_LEAVE_PARTY, "TARGET_BUTTON_LEAVE_PARTY"),
		(BTN_EXCLUDE, "TARGET_BUTTON_EXCLUDE"),
		(BTN_INVITE_GUILD, "TARGET_BUTTON_INVITE_GUILD"),
		(BTN_REMOVE_GUILD, "TARGET_BUTTON_REMOVE_GUILD"),
		(BTN_DISMOUNT, "TARGET_BUTTON_DISMOUNT"),
		(BTN_EXIT_OBSERVER, "TARGET_BUTTON_EXIT_OBSERVER"),
		(BTN_VIEW_EQUIPMENT, "TARGET_BUTTON_VIEW_EQUIPMENT"),
		(BTN_REQUEST_ENTER_PARTY, "TARGET_BUTTON_REQUEST_ENTER_PARTY"),
		(BTN_BUILDING_DESTROY, "TARGET_BUTTON_BUILDING_DESTROY"),
		(BTN_EMOTION_ALLOW, "TARGET_BUTTON_EMOTION_ALLOW"),
		(BTN_VOTE_BLOCK_CHAT, "VOTE_BLOCK_CHAT"),
	)

	GRADE_NAME = {}
	EXCHANGE_LIMIT_RANGE = 3000

	@staticmethod
	def _RebuildLocaleStrings():
		TargetBoard.GRADE_NAME = {
			nonplayer.PAWN : localeInfo.TARGET_LEVEL_PAWN,
			nonplayer.S_PAWN : localeInfo.TARGET_LEVEL_S_PAWN,
			nonplayer.KNIGHT : localeInfo.TARGET_LEVEL_KNIGHT,
			nonplayer.S_KNIGHT : localeInfo.TARGET_LEVEL_S_KNIGHT,
			nonplayer.BOSS : localeInfo.TARGET_LEVEL_BOSS,
			nonplayer.KING : localeInfo.TARGET_LEVEL_KING,
		}

	def __init__(self):
		ui.ThinBoard.__init__(self)

		name = ui.TextLine()
		name.SetParent(self)
		name.SetDefaultFontName()
		name.SetOutline()
		name.Show()

		hpGauge = ui.Gauge()
		hpGauge.SetParent(self)
		hpGauge.MakeGauge(130, "red")
		hpGauge.Hide()

		closeButton = ui.Button()
		closeButton.SetParent(self)
		closeButton.SetUpVisual("d:/ymir work/ui/public/close_button_01.sub")
		closeButton.SetOverVisual("d:/ymir work/ui/public/close_button_02.sub")
		closeButton.SetDownVisual("d:/ymir work/ui/public/close_button_03.sub")
		closeButton.SetPosition(30, 13)

		if app.IsRTL():
			hpGauge.SetPosition(55, 17)
			hpGauge.SetWindowHorizontalAlignLeft()
			closeButton.SetWindowHorizontalAlignLeft()
		else:
			hpGauge.SetPosition(175, 17)
			hpGauge.SetWindowHorizontalAlignRight()
			closeButton.SetWindowHorizontalAlignRight()

		closeButton.SetEvent(ui.__mem_func__(self.OnPressedCloseButton))
		closeButton.Show()

		self.buttonDict = {}
		self.showingButtonList = []

		for btnId, localeAttr in self.BUTTON_CONFIG:
			button = ui.Button()
			button.SetParent(self)

			if app.IsRTL():
				button.SetUpVisual("d:/ymir work/ui/public/Small_Button_01.sub")
				button.SetOverVisual("d:/ymir work/ui/public/Small_Button_02.sub")
				button.SetDownVisual("d:/ymir work/ui/public/Small_Button_03.sub")
			else:
				button.SetUpVisual("d:/ymir work/ui/public/small_thin_button_01.sub")
				button.SetOverVisual("d:/ymir work/ui/public/small_thin_button_02.sub")
				button.SetDownVisual("d:/ymir work/ui/public/small_thin_button_03.sub")

			button.SetWindowHorizontalAlignCenter()
			button.SetText(getattr(localeInfo, localeAttr, localeAttr))
			button.Hide()
			self.buttonDict[btnId] = button
			self.showingButtonList.append(button)

		self.buttonDict[self.BTN_WHISPER].SetEvent(ui.__mem_func__(self.OnWhisper))
		self.buttonDict[self.BTN_EXCHANGE].SetEvent(ui.__mem_func__(self.OnExchange))
		self.buttonDict[self.BTN_FIGHT].SetEvent(ui.__mem_func__(self.OnPVP))
		self.buttonDict[self.BTN_ACCEPT_FIGHT].SetEvent(ui.__mem_func__(self.OnPVP))
		self.buttonDict[self.BTN_AVENGE].SetEvent(ui.__mem_func__(self.OnPVP))
		self.buttonDict[self.BTN_FRIEND].SetEvent(ui.__mem_func__(self.OnAppendToMessenger))
		self.buttonDict[self.BTN_INVITE_PARTY].SetEvent(ui.__mem_func__(self.OnPartyInvite))
		self.buttonDict[self.BTN_LEAVE_PARTY].SetEvent(ui.__mem_func__(self.OnPartyExit))
		self.buttonDict[self.BTN_EXCLUDE].SetEvent(ui.__mem_func__(self.OnPartyRemove))

		self.buttonDict[self.BTN_INVITE_GUILD].SAFE_SetEvent(self.__OnGuildAddMember)
		self.buttonDict[self.BTN_REMOVE_GUILD].SAFE_SetEvent(self.__OnGuildRemoveMember)
		self.buttonDict[self.BTN_DISMOUNT].SAFE_SetEvent(self.__OnDismount)
		self.buttonDict[self.BTN_EXIT_OBSERVER].SAFE_SetEvent(self.__OnExitObserver)
		self.buttonDict[self.BTN_VIEW_EQUIPMENT].SAFE_SetEvent(self.__OnViewEquipment)
		self.buttonDict[self.BTN_REQUEST_ENTER_PARTY].SAFE_SetEvent(self.__OnRequestParty)
		self.buttonDict[self.BTN_BUILDING_DESTROY].SAFE_SetEvent(self.__OnDestroyBuilding)
		self.buttonDict[self.BTN_EMOTION_ALLOW].SAFE_SetEvent(self.__OnEmotionAllow)

		self.buttonDict[self.BTN_VOTE_BLOCK_CHAT].SetEvent(ui.__mem_func__(self.__OnVoteBlockChat))

		self.name = name
		self.hpGauge = hpGauge
		self.closeButton = closeButton
		self.nameString = 0
		self.nameLength = 0
		self.vid = 0
		self.eventWhisper = None
		self.isShowButton = False

		self.__Initialize()
		self.ResetTargetBoard()

	def __del__(self):
		ui.ThinBoard.__del__(self)

		print("===================================================== DESTROYED TARGET BOARD")

	def __Initialize(self):
		self.nameString = ""
		self.nameLength = 0
		self.vid = 0
		self.isShowButton = False

	def Destroy(self):
		self.eventWhisper = None
		self.closeButton = None
		self.showingButtonList = None
		self.buttonDict = None
		self.name = None
		self.hpGauge = None
		self.__Initialize()

	def OnPressedCloseButton(self):
		player.ClearTarget()
		self.Close()

	def Close(self):
		self.__Initialize()
		self.Hide()

	def Open(self, vid, name):
		if vid:
			if not constInfo.GET_VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD():
				if not player.IsSameEmpire(vid):
					self.Hide()
					return

			if vid != self.GetTargetVID():
				self.ResetTargetBoard()
				self.SetTargetVID(vid)
				self.SetTargetName(name)

			if player.IsMainCharacterIndex(vid):
				self.__ShowMainCharacterMenu()		
			elif chr.INSTANCE_TYPE_BUILDING == chr.GetInstanceType(self.vid):
				self.Hide()
			else:
				self.RefreshButton()
				self.Show()
		else:
			self.HideAllButton()
			self.__ShowButton(self.BTN_WHISPER)
			self.__ShowButton(self.BTN_VOTE_BLOCK_CHAT)
			self.__ArrangeButtonPosition()
			self.SetTargetName(name)
			self.Show()
			
	def Refresh(self):
		if self.IsShow():
			if self.IsShowButton():			
				self.RefreshButton()		

	def RefreshByVID(self, vid):
		if vid == self.GetTargetVID():			
			self.Refresh()
			
	def RefreshByName(self, name):
		if name == self.GetTargetName():
			self.Refresh()

	def __ShowMainCharacterMenu(self):
		canShow=0

		self.HideAllButton()

		if player.IsMountingHorse():
			self.__ShowButton(self.BTN_DISMOUNT)
			canShow=1

		if player.IsObserverMode():
			self.__ShowButton(self.BTN_EXIT_OBSERVER)
			canShow=1

		if canShow:
			self.__ArrangeButtonPosition()
			self.Show()
		else:
			self.Hide()
			
	def __ShowNameOnlyMenu(self):
		self.HideAllButton()

	def SetWhisperEvent(self, event):
		self.eventWhisper = event

	def UpdatePosition(self):
		self.SetPosition(wndMgr.GetScreenWidth() // 2 - self.GetWidth() // 2, 10)

	def ResetTargetBoard(self):

		for btn in list(self.buttonDict.values()):
			btn.Hide()

		self.__Initialize()

		self.name.SetPosition(0, 13)
		self.name.SetHorizontalAlignCenter()
		self.name.SetWindowHorizontalAlignCenter()
		self.hpGauge.Hide()
		self.SetSize(250, 40)

	def SetTargetVID(self, vid):
		self.vid = vid

	def SetEnemyVID(self, vid):
		self.SetTargetVID(vid)

		name = chr.GetNameByVID(vid)
		level = nonplayer.GetLevelByVID(vid)
		grade = nonplayer.GetGradeByVID(vid)

		nameFront = ""
		if -1 != level:
			nameFront += "Lv." + str(level) + " "
		if grade in self.GRADE_NAME:
			nameFront += "(" + self.GRADE_NAME[grade] + ") "

		self.SetTargetName(nameFront + name)

	def GetTargetVID(self):
		return self.vid

	def GetTargetName(self):
		return self.nameString

	def SetTargetName(self, name):
		self.nameString = name
		self.nameLength = len(name)
		self.name.SetText(name)

	def SetHP(self, hpPercentage):
		if not self.hpGauge.IsShow():

			self.SetSize(200 + 7 * self.nameLength, self.GetHeight())

			if app.IsRTL():
				self.name.SetPosition( self.GetWidth()-23, 13)
			else:
				self.name.SetPosition(23, 13)

			self.name.SetWindowHorizontalAlignLeft()
			self.name.SetHorizontalAlignLeft()
			self.hpGauge.Show()
			self.UpdatePosition()

		self.hpGauge.SetPercentage(hpPercentage, 100)

	def ShowDefaultButton(self):

		self.isShowButton = True
		self.showingButtonList.append(self.buttonDict[self.BTN_WHISPER])
		self.showingButtonList.append(self.buttonDict[self.BTN_EXCHANGE])
		self.showingButtonList.append(self.buttonDict[self.BTN_FIGHT])
		self.showingButtonList.append(self.buttonDict[self.BTN_EMOTION_ALLOW])
		for button in self.showingButtonList:
			button.Show()

	def HideAllButton(self):
		self.isShowButton = False
		for button in self.showingButtonList:
			button.Hide()
		self.showingButtonList = []

	def __ShowButton(self, name):

		if name not in self.buttonDict:
			return

		self.buttonDict[name].Show()
		self.showingButtonList.append(self.buttonDict[name])

	def __HideButton(self, name):

		if name not in self.buttonDict:
			return

		button = self.buttonDict[name]
		button.Hide()

		for btnInList in self.showingButtonList:
			if btnInList == button:
				self.showingButtonList.remove(button)
				break

	def OnWhisper(self):
		if None != self.eventWhisper:
			self.eventWhisper(self.nameString)

	def OnExchange(self):
		net.SendExchangeStartPacket(self.vid)

	def OnPVP(self):
		net.SendChatPacket("/pvp %d" % (self.vid))

	def OnAppendToMessenger(self):
		net.SendMessengerAddByVIDPacket(self.vid)

	def OnPartyInvite(self):
		net.SendPartyInvitePacket(self.vid)

	def OnPartyExit(self):
		net.SendPartyExitPacket()

	def OnPartyRemove(self):
		net.SendPartyRemovePacket(self.vid)

	def __OnGuildAddMember(self):
		net.SendGuildAddMemberPacket(self.vid)

	def __OnGuildRemoveMember(self):
		self.questionDialog = uiCommon.QuestionDialog()
		self.questionDialog.SetText(localeInfo.GUILD_REMOVE_MEMBER_QUESTION)
		self.questionDialog.SetAcceptEvent(ui.__mem_func__(self.__OnGuildRemoveMemberAccept))
		self.questionDialog.SetCancelEvent(ui.__mem_func__(self.__OnGuildRemoveMemberClose))
		self.questionDialog.Open()

	def __OnGuildRemoveMemberAccept(self):
		net.SendGuildRemoveMemberPacket(self.nameString)
		self.__OnGuildRemoveMemberClose()
  
	def __OnGuildRemoveMemberClose(self):
		self.questionDialog.Close()
		self.questionDialog = None
		return True

	def __OnDismount(self):
		net.SendChatPacket("/unmount")

	def __OnExitObserver(self):
		net.SendChatPacket("/observer_exit")

	def __OnViewEquipment(self):
		net.SendChatPacket("/view_equip " + str(self.vid))

	def __OnRequestParty(self):
		net.SendChatPacket("/party_request " + str(self.vid))

	def __OnDestroyBuilding(self):
		net.SendChatPacket("/build d %d" % (self.vid))

	def __OnEmotionAllow(self):
		net.SendChatPacket("/emotion_allow %d" % (self.vid))
		
	def __OnVoteBlockChat(self):
		cmd = "/vote_block_chat %s" % (self.nameString)
		net.SendChatPacket(cmd)

	def OnPressEscapeKey(self):
		self.OnPressedCloseButton()
		return True

	def IsShowButton(self):
		return self.isShowButton

	def RefreshButton(self):
		self.HideAllButton()

		if chr.INSTANCE_TYPE_BUILDING == chr.GetInstanceType(self.vid):
			#self.__ShowButton(self.BTN_BUILDING_DESTROY)
			#self.__ArrangeButtonPosition()
			return
		
		if player.IsPVPInstance(self.vid) or player.IsObserverMode():
			# PVP_INFO_SIZE_BUG_FIX
			self.SetSize(200 + 7 * self.nameLength, 40)
			self.UpdatePosition()
			# END_OF_PVP_INFO_SIZE_BUG_FIX			
			return	

		self.ShowDefaultButton()

		def isGuildMaster(name):
			guildMasterName = guild.GetGuildMasterName()
			return guildMasterName == name

		def isGuildMember(name, vid):
			return guild.IsMemberByName(name) and chr.GetGuildID(vid) != 0
	
		def isNotGuildMember(name, vid):
			return not guild.IsMemberByName(name) and chr.GetGuildID(vid) == 0
  
		guildAuthorityButtons = {
			guild.AUTH_ADD_MEMBER: {
				"btn": self.BTN_INVITE_GUILD,
				"condition": lambda: isNotGuildMember(self.nameString, self.vid),
			},
			guild.AUTH_REMOVE_MEMBER: {
				"btn": self.BTN_REMOVE_GUILD,
				"condition": lambda: isGuildMember(self.nameString, self.vid) and not isGuildMaster(self.nameString),
			},
		}

		for guildAuthority, guildButton in guildAuthorityButtons.items():
			hasAuthority = guild.MainPlayerHasAuthority(guildAuthority)
			satisfiesCondition = guildButton["condition"]()
			if hasAuthority and satisfiesCondition:
				self.__ShowButton(guildButton["btn"])

		if not messenger.IsFriendByName(self.nameString):
			self.__ShowButton(self.BTN_FRIEND)

		if player.IsPartyMember(self.vid):

			self.__HideButton(self.BTN_FIGHT)

			if player.IsPartyLeader(self.vid):
				self.__ShowButton(self.BTN_LEAVE_PARTY)
			elif player.IsPartyLeader(player.GetMainCharacterIndex()):
				self.__ShowButton(self.BTN_EXCLUDE)

		else:
			if player.IsPartyMember(player.GetMainCharacterIndex()):
				if player.IsPartyLeader(player.GetMainCharacterIndex()):
					self.__ShowButton(self.BTN_INVITE_PARTY)
			else:
				if chr.IsPartyMember(self.vid):
					self.__ShowButton(self.BTN_REQUEST_ENTER_PARTY)
				else:
					self.__ShowButton(self.BTN_INVITE_PARTY)

			if player.IsRevengeInstance(self.vid):
				self.__HideButton(self.BTN_FIGHT)
				self.__ShowButton(self.BTN_AVENGE)
			elif player.IsChallengeInstance(self.vid):
				self.__HideButton(self.BTN_FIGHT)
				self.__ShowButton(self.BTN_ACCEPT_FIGHT)
			elif player.IsCantFightInstance(self.vid):
				self.__HideButton(self.BTN_FIGHT)

			if not player.IsSameEmpire(self.vid):
				self.__HideButton(self.BTN_INVITE_PARTY)
				self.__HideButton(self.BTN_FRIEND)
				self.__HideButton(self.BTN_FIGHT)

		distance = player.GetCharacterDistance(self.vid)
		if distance > self.EXCHANGE_LIMIT_RANGE:
			self.__HideButton(self.BTN_EXCHANGE)
			self.__ArrangeButtonPosition()

		self.__ArrangeButtonPosition()

	def __ArrangeButtonPosition(self):
		showingButtonCount = len(self.showingButtonList)

		pos = -(showingButtonCount // 2) * 68

		if 0 == showingButtonCount % 2:
			pos += 34

		for button in self.showingButtonList:
			button.SetPosition(pos, 33)
			pos += 68

		self.SetSize(max(150, showingButtonCount * 75), 65)
		self.UpdatePosition()

	def OnUpdate(self):
		if self.isShowButton:

			exchangeButton = self.buttonDict[self.BTN_EXCHANGE]
			distance = player.GetCharacterDistance(self.vid)

			if distance < 0:
				return

			if exchangeButton.IsShow():
				if distance > self.EXCHANGE_LIMIT_RANGE:
					self.RefreshButton()

			else:
				if distance < self.EXCHANGE_LIMIT_RANGE:
					self.RefreshButton()

TargetBoard._RebuildLocaleStrings()
localeInfo.RegisterReloadCallback(TargetBoard._RebuildLocaleStrings)
