
***<span
style="background: #f8fafe"><font size = 6>智能合约API—LUA</font></span>***


#<span lang="zh-CN">目录</span>

##[1虚拟机层跟中间层接口](#__RefHeading__4350_2146186683)

###[1.1数据保存格式](#__RefHeading__4352_2146186683) 

###[1.2数据传输协议](#__RefHeading__4354_2146186683) 

###[1.3函数传输协议](#__RefHeading__4356_2146186683) 

##[2可校验的规则](#__RefHeading__4358_2146186683) 

##[3公共](#__RefHeading__4360_2146186683) 

###[3.1常量](#__RefHeading__4362_2146186683)

###[3.2公共函数](#__RefHeading__4364_2146186683)

##[4系统设备环境](#__RefHeading__4366_2146186683) 

###[4.1GetTxAccounts](#__RefHeading__4368_2146186683)

###[4.2GetAccountPublickey](#__RefHeading__4370_2146186683)

###[4.3QueryAccountBalance](#__RefHeading__4372_2146186683)

###[4.4GetTxConFirmHeight](#__RefHeading__4374_2146186683)

###[4.5GetBlockHash(height)](#__RefHeading__4376_2146186683)

###[4.6GetCurRunEnvHeight](#__RefHeading__4378_2146186683)

###[4.7WriteData ](#__RefHeading__4380_2146186683)

###[4.8DeleteData ](#__RefHeading__4382_2146186683)

###[4.9ReadData ](#__RefHeading__4384_2146186683)

###[4.10ModifyData ](#__RefHeading__4386_2146186683)

###[4.11LogPrint ](#__RefHeading__4388_2146186683)

###[4.12GetCurTxHash ](#__RefHeading__4390_2146186683)

###[4.13WriteOutput ](#__RefHeading__4392_2146186683)

###[4.14GetScriptData ](#__RefHeading__4394_2146186683)

###[4.15GetScriptID ](#__RefHeading__4396_2146186683)

###[4.16GetCurTxAccount ](#__RefHeading__4398_2146186683)

###[4.17GetCurTxPayAmount ](#__RefHeading__4400_2146186683)

###[4.18GetUserAppAccValue ](#__RefHeading__4402_2146186683)

###[4.19GetUserAppAccFoudWithTag ](#__RefHeading__4404_2146186683)

###[4.20WriteOutAppOperate ](#__RefHeading__4406_2146186683)

###[4.21GetBase58Addr ](#__RefHeading__4408_2146186683)

###[4.22ByteToInteger ](#__RefHeading__4410_2146186683)

###[4.23IntegerToByte4 ](#__RefHeading__4412_2146186683)

###[4.24IntegerToByte8 ](#__RefHeading__4414_2146186683)

###[4.25Sha256 ](#__RefHeading__4416_2146186683)

###[4.26Des ](#__RefHeading__4418_2146186683)

###[4.27VerifySignature ](#__RefHeading__4420_2146186683)

###[4.28GetTxContracts ](#__RefHeading__4422_2146186683)

###[4.29TransferContactAsset ](#__RefHeading__4424_2146186683)

###[4.30TransferSomeAsset ](#__RefHeading__4426_2146186683)

###[4.31GetBlockTimestamp ](#__RefHeading__4428_2146186683)


<h2 id=__RefHeading__4350_2146186683>1虚拟机层跟中间层接口</h2>
<h3 id=__RefHeading__4352_2146186683>1.1数据保存格式</h3>

<span
lang="zh-CN">所有在此虚拟机上执行有关于数据的保存，两个字节以及四个字节的都是低位保存在前高位保存在后。</span>



<h3 id=__RefHeading__4354_2146186683>1.2数据传输协议</h3>

虚拟机请求数据：
<table width="572" cellpadding="7" cellspacing="0">
	<col width="176">
	<col width="177">
	<col width="176">
	<tr valign="top">
		<td width="176" height="10" bgcolor="#acb9ca" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center"><font face="Times New Roman"><span lang="zh-CN"><b>标示</b></span></font></p>
		</td>
		<td width="177" bgcolor="#acb9ca" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center"><font face="Times New Roman"><span lang="zh-CN"><b>数据长度</b></span></font></p>
		</td>
		<td width="176" bgcolor="#acb9ca" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center"><font face="Times New Roman"><span lang="zh-CN"><b>数据</b></span></font></p>
		</td>
	</tr>
	<tr valign="top">
		<td width="176" height="11" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center" style="margin-bottom: 0in"><font face="Times New Roman, serif">2</font><font face="Times New Roman"><span lang="zh-CN">个字节</span></font></p>
			<p align="center" style="margin-bottom: 0in"><font face="Times New Roman"><span lang="zh-CN">低位在前高位在后</span></font></p>
			<p align="center" style="margin-bottom: 0in"><font face="Times New Roman, serif">Forexample:5000</font></p>
			<p align="center"><font face="Times New Roman"><span lang="zh-CN">内存表示</span></font><font face="Times New Roman, serif">:0x88
			0x13</font></p>
		</td>
		<td width="177" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center" style="margin-bottom: 0in"><font face="Times New Roman, serif">2</font><font face="Times New Roman"><span lang="zh-CN">个字节</span></font></p>
			<p align="center"><font face="Times New Roman"><span lang="zh-CN">低位在前高位在后</span></font></p>
		</td>
		<td width="176" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center"><font face="Times New Roman"><span lang="zh-CN">二进制数据</span></font><font face="Times New Roman, serif">:0x56
			0x23 0x10</font></p>
		</td>
	</tr>
</table>

<br/>

<span
lang="zh-CN">中间层响应数据，解析请求的数据格式，执行指令，返回数据</span>(<span
lang="zh-CN">长度</span>+<span lang="zh-CN">数据</span>)
<table width="577" cellpadding="7" cellspacing="0">
	<col width="274">
	<col width="274">
	<tr valign="top">
		<td width="274" height="7" bgcolor="#acb9ca" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center"><font face="Times New Roman"><span lang="zh-CN"><b>数据长度</b></span></font></p>
		</td>
		<td width="274" bgcolor="#acb9ca" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center"><font face="Times New Roman"><span lang="zh-CN"><b>数据</b></span></font></p>
		</td>
	</tr>
	<tr valign="top">
		<td width="274" height="29" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center" style="margin-bottom: 0in"><font face="Times New Roman, serif">2</font><font face="Times New Roman"><span lang="zh-CN">个字节</span></font></p>
			<p align="center"><font face="Times New Roman"><span lang="zh-CN">低位在前高位在后</span></font></p>
		</td>
		<td width="274" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
			<p align="center"><font face="Times New Roman"><span lang="zh-CN">二进制数据</span></font></p>
		</td>
	</tr>
</table>


<h3 id=__RefHeading__4356_2146186683>1.3函数传输协议</h3>

API<span lang="zh-CN">接口函数，与宿主语言</span>C<span
lang="zh-CN">之间的交互都是通过</span>lua<span
lang="zh-CN">解释器自带的虚拟栈实现的。</span>

<h2 id=__RefHeading__4358_2146186683>2 可校验的规则</h2>

<span><font size = 4>1 检查此交易输出指令加减金额是否平衡。</font></span>

<span><font size = 4>2 从冻结金额里面扣钱，超时高度必须大于当前tip高度。</font></span>

<span><font size = 4>3 当前交易不能扣其他脚本账户中的金额。</font></span>

<span><font size = 4>4 加班账户的操作只能是价钱到自由金额或者从自由金额中减钱。</font></span>

<span><font size = 4>5 授权期限是否超时。</font></span>

<span><font size = 4>6 当前没有跨天，检查要扣的钱是否超过当前能花费最大余额。</font></span>

<span><font size = 4>7 当前有跨天，则检查要扣的钱是否超过一天最大允许花费余额。</font></span>

<span><font size = 4>8 检查要扣的钱是否超过一次最大可消费金额。</font></span>

<span><font size = 4>9 检查要扣的钱是否超过最大可消费金额。</font></span>

<br/>


<h2 id=__RefHeading__4360_2146186683>3 公共</span></h2>

<span lang="zh-CN">使用的全局常量</span>

<h3 id=__RefHeading__4362_2146186683>3.1 常量</h3>

--<span lang="zh-CN">脚本应用账户操作类型定义</span>

APP\_OPERATOR\_TYPE =

{

ENUM\_ADD\_FREE\_OP = 1, --<span lang="zh-CN">自由账户加</span>

ENUM\_SUB\_FREE\_OP = 2, --<span lang="zh-CN">自由账户减</span>

ENUM\_ADD\_FREEZED\_OP = 3, --<span lang="zh-CN">冻结账户加</span>

ENUM\_SUB\_FREEZED\_OP = 4 --<span lang="zh-CN">冻结账户减</span>

}

--<span lang="zh-CN">系统账户操作定义</span>

OPER\_TYPE =

{

ENUM\_ADD\_FREE = 1, --<span lang="zh-CN">系统账户加</span>

ENUM\_MINUS\_FREE = 2 --<span lang="zh-CN">系统账户减</span>

}

--<span lang="zh-CN">账户地址类型定义</span>

ADDR\_TYPE =

{

ENUM\_REGID = 1, -- REG\_ID

ENUM\_BASE58 = 2 -- BASE58 ADDR

}

--<span lang="zh-CN">日志输出类型</span>

LOG\_TYPE =

{

ENUM\_STRING = 0, --<span lang="zh-CN">字符串类型</span>

ENUM\_NUMBER = 1 --<span lang="zh-CN">数字类型 </span>

}

<h3 id=__RefHeading__4364_2146186683>3.2 公共函数</h3>

--<span lang="zh-CN">输出</span>table<span
lang="zh-CN">数组所有元素</span>

function Unpack(t,i)

i = i or 1

if t\[i\] then

return t\[i\],Unpack(t,i+1)

end

end

<br/>

--<span lang="zh-CN">检测一个表是否为空表</span>

function TableIsEmpty(t)

return \_G.next(t) == nil

end

<br/>
--<span lang="zh-CN">检测一个表是否为非空表</span>

function TableIsNotEmpty(t)

return false == TableIsEmpty(t)

end

<br/>

--<span lang="zh-CN">输出日志信息</span>

--aKey <span lang="zh-CN">输入日志类型 字符串或数字</span>

--bLength <span lang="zh-CN">打印输出信息的长度</span>

--cValue <span
lang="zh-CN">待输出的信息，字符串类型就是一串字符串，数字类型就是数组</span>

function LogPrint(aKey,bLength,cValue)

assert(bLength &gt;= 1,"LogPrint bLength invlaid")

if(aKey == LOG\_TYPE.ENUM\_STRING) then

assert(type(cValue) == "string","LogPrint cValue invlaid0")

elseif(aKey == LOG\_TYPE.ENUM\_NUMBER) then

assert(TableIsNotEmpty(cValue),"LogPrint cValue invlaid1")

else

error("LogPrint aKey invlaid")

end

<br/>

local logTable = {

key = LOG\_TYPE.ENUM\_STRING,

length = 0,

value = nil

}

logTable.key = aKey

logTable.length = bLength

logTable.value = cValue

mylib.LogPrint(logTable)

end

<br/>
-- <span lang="zh-CN">打印操作账户的信息</span>

function PrintAppOperate(appOperateTbl)

local locValue = {}

LogPrint(LOG\_TYPE. ENUM\_STRING,21,"PrintAppOperate start")

LogPrint(LOG\_TYPE. ENUM\_STRING,8,"AppAccID")

LogPrint(LOG\_TYPE. ENUM\_NUMBER, appOperateTbl.useridlen,
appOperateTbl.userIdTbl)

LogPrint(LOG\_TYPE. ENUM\_STRING,3,"tag")

if (appOperateTbl.fundTaglen &gt; 0) then

LogPrint(LOG\_TYPE. ENUM\_NUMBER, appOperateTbl.fundTaglen,
appOperateTbl.fundTagTbl)

end

LogPrint(LOG\_TYPE. ENUM\_STRING,11," operatorType")

locValue = { appOperateTbl.operatorType}

LogPrint(LOG\_TYPE. ENUM\_NUMBER,1,locValue)

LogPrint(LOG\_TYPE. ENUM\_STRING,9,"outheight")

locValue = nil

locValue = {mylib.IntToByte(appOperateTbl.outHeight)}

LogPrint(LOG\_TYPE. ENUM\_NUMBER,\#locValue,locValue)

LogPrint(LOG\_TYPE. ENUM\_STRING,6,"mMoney")

LogPrint(LOG\_TYPE. ENUM\_NUMBER,8, appOperateTbl.moneyTbl)

LogPrint(LOG\_TYPE. ENUM\_STRING,19,"PrintAppOperate end")

end

<h2 id=__RefHeading__4366_2146186683>4 系统设备环境</h2>

<span
lang="zh-CN">本模块主要提供操作系统账户，脚本账户等系统侧的</span>API<span
lang="zh-CN">接口函数</span>,<span
lang="zh-CN">所有的接口函数都封装在</span>mylib<span
lang="zh-CN">库中。**系统内相关函数如下：**</span>


<h3 id=__RefHeading__4368_2146186683>4.1 GetTxAccounts</span></h3> 
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetTxAccounts(hashTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取指定</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">hash</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">的交易账户</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">交易</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">hash</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：返回交易账户；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ GetTxAccounts ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				hash = {0x24, 0x4f, 0xa7, 0xcf, 0x97, 0xae, 0x15, 0x85, 0xd8,
				0xf8, 0x02, 0x4b, 0xa1, 0x8b, 0x8a, 0xbe, 0xce,   0x8e, 0xb9,
				0xcd, 0x4d, 0x01, 0x6d, 0xd0, 0xba, 0x8c, 0xc0, 0xdc, 0x85, 0x1a,
				0x9c, 0x0e}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				</font></font><font color="#000000"><font size="2" style="font-size: 9pt">accounts</font></font><font color="#000000"><font size="2" style="font-size: 9pt">
				= {mylib.GetTxAccounts(Unpack(hash))}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">LogPrint(LOG_TYPE.ENUM_NUMBER,#</font></font><font color="#000000"><font size="2" style="font-size: 9pt">
				accounts</font></font><font color="#000000"><font size="2" style="font-size: 9pt">,</font></font><font color="#000000"><font size="2" style="font-size: 9pt">
				accounts</font></font><font color="#000000"><font size="2" style="font-size: 9pt">)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
</div>


<h3 id=__RefHeading__4370_2146186683>4.2 GetAccountPublickey</span></h3>  

<div><center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetAccountPublickey(accountTbl[1],accountTbl[2],accountTbl[3],accountTbl[4],accountTbl[5],accountTbl[6])</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取账户的公钥</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				accountTbl = {mylib.GetCurTxAccount()}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：账户的</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">publickey</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetAccountPublickey()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				accountTbl = {mylib.GetCurTxAccount()}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				result =
				{mylib.GetAccountPublickey(accountTbl[1],accountTbl[2],accountTbl[3],</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">                                       
				<font size="2" style="font-size: 9pt">accountTbl[4],accountTbl[5],accountTbl[6])}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#result
				== 33,&quot;GetAccountPublickey err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;Publickey&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
			</td>
		</tr>
	</table>
</center></div>

<h3 id=__RefHeading__4372_2146186683>4.3 QueryAccountBalance</span></h3> 
<div><center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.QueryAccountBalance(unpack(accountTbl))</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取账户的余额</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				accountTbl = {mylib.GetCurTxAccount()}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：账户的余额；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_QueryAccountBalance()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">     <font size="2" style="font-size: 9pt">local
				accountTbl = {mylib.GetCurTxAccount()}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				result = {mylib.QueryAccountBalance(unpack(accountTbl))}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#result
				== 8,&quot;QueryAccountBalance err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">     <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	  
				print(&quot;Balance&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">     <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%"></div>

<h3 id=__RefHeading__4374_2146186683>4.4 GetTxConFirmHeight</h3>  
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetTxConFirmHeight(unpack(txhashTbl))</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取指定</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">hash</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">交易的确认高度</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">交易</font></font></span></font><font face="Times New Roman, serif"><font color="#000000">hash</font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：交易确认高度；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetTxConFirmHeight()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">--
				&quot;hash&quot;
				:&quot;4a2af2d83683325e780f2b859e7421f4592e3105d01017aab45c15da3910be8e&quot;</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				txhashTbl = {0x4a,0x2a,0xf2,0xd8,0x36,0x83,0x32,0x5e,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">                               
				<font size="2" style="font-size: 9pt">0x78,0x0f,0x2b,0x85,0x9e,0x74,0x21,0xf4,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">                               
				<font size="2" style="font-size: 9pt">0x59,0x2e,0x31,0x05,0xd0,0x10,0x17,0xaa,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">                               
				<font size="2" style="font-size: 9pt">0xb4,0x5c,0x15,0xda,0x39,0x10,0xbe,0x8e}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">local
				result = mylib.GetTxConFirmHeight(unpack(txhashTbl)) </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">assert(result
				&gt; 0,&quot;GetTxConFirmHeight err&quot;);    </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4376_2146186683>4.5 GetBlockHash(height)</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetBlockHash(height)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取指定块</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">hash
				</font></font>
				</p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">块的高度</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：块</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">hash</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">值；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetBlockHash()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				height = 47037</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				result = {mylib.GetBlockHash(height)}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">assert(#result
				== 32,&quot;GetBlockHash err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;BlockHash&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4378_2146186683>4.6 GetCurRunEnvHeight</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetCurRunEnvHeight()</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取当前运行高度
				</font></font></span></font>
				</p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：块的高度；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetCurRunEnvHeight()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				result = mylib.GetCurRunEnvHeight()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">assert(result
				&gt; 0,&quot;GetCurRunEnvHeight err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">print(&quot;RunEnvHeight&quot;,result)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end
				</font></font>
				</p>
			</td>
		</tr>
	</table>
</center>
</div>

<h3 id=__RefHeading__4380_2146186683>4.7 WriteData</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.WriteData(writeDbTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">写脚本数据库</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				writeDbTbl = {</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	
				 key = &quot;config&quot;,  --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">关键字</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	
				 length = 0,  -- value</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">数据流的总长</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	
				 value = {} --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">值</font></font></span></font></p>
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：执行成功；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_WriteData()	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">local
				writeDbTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				key = &quot;config&quot;,  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">关键字</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				length = 0,  -- value</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">数据流的总长</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				value = {} --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">值</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	addressTbl
				= {0x64,0x63,0x6D,0x43,0x62,0x4B,0x62,0x41,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">					0x66,0x4B,0x72,0x6F,0x66,0x4E,0x7A,0x33,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">					0x35,0x4D,0x53,0x46,0x75,0x70,0x78,0x72,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">					0x78,0x34,0x55,0x77,0x6E,0x33,0x76,0x67,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">					0x6A,0x4C}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	--</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">每次限额</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">,</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">每日限额</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">,</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">企业登记的账户地址</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	writeDbTbl.value
				= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	    
				               0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x2C,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">						unpack(addressTbl)}				</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	writeDbTbl.length
				= #writeDbTbl.value</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   
				<font size="2" style="font-size: 9pt">assert(mylib.WriteData(writeDbTbl),&quot;WriteData
				err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
</div>

<h3 id=__RefHeading__4382_2146186683>4.8 DeleteData</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.DeleteData(key)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">删除脚本数据库</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000">关键字</font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif">key
				=“config”</font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">true</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">false</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ DeleteData ()	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">local
				writeDbTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				key = &quot;config&quot;,  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">关键字</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				length = 0,  -- value</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">数据流的总长</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				value = {} --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">值</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(mylib.DeleteData(writeDbTbl.key),&quot;DeleteData
				err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	print(&quot;DeleteData
				return ok&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				readResult = {}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	readResult
				=  {mylib.ReadData(writeDbTbl.key)}	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	if(TableIsEmpty(readResult))
				then</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		print(&quot;DeleteData
				ok&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	end	</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4384_2146186683>4.9 ReadData</span></h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.ReadData(key)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">根据指定的</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">key</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">读取脚本数据库中的记录</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">关键字</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">key
				=“config”</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：返回</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">value</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ ReadData ()	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">local
				key =“config”</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">local
				readResult =  {mylib.ReadData(key)}	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">assert(#readResult
				&gt; 0,&quot;ReadData0 err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">for
				i = 1,#readResult do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	print(&quot;&quot;,i,(readResult[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">end
				</font></font>
				</p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4386_2146186683>4.10 ModifyData</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.ModifyData(writeDbTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">修改脚本数据库内容</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				writeDbTbl = {</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	
				 key = &quot;config&quot;,  --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">关键字</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	
				 length = 0,  -- value</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">数据流的总长</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	
				 value = {} --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">值</font></font></span></font></p>
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">True</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">false</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ ModifyData ()	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">local
				writeDbTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				key = &quot;config&quot;,  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">关键字</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				length = 0,  -- value</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">数据流的总长</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				value = {} --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">值</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	addressTbl
				= {0x64,0x63,0x6D,0x43,0x62,0x4B,0x62,0x41,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">					0x66,0x4B,0x72,0x6F,0x66,0x4E,0x7A,0x33,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">					0x35,0x4D,0x53,0x46,0x75,0x70,0x78,0x72,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">					0x78,0x34,0x55,0x77,0x6E,0x33,0x76,0x67,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">					0x6A,0x4C}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	--</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">每次限额</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">,</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">每日限额</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">,</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">企业登记的账户地址</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	writeDbTbl.value
				= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	    
				               0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x2C,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">						unpack(addressTbl)}			</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	writeDbTbl.length
				= #writeDbTbl.value</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	writeDbTbl.value[8]
				= 200</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(mylib.ModifyData(writeDbTbl),&quot;ModifyData
				err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	print(&quot;ModifyData
				ok&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	readResult
				= {}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	readResult
				=  {mylib.ReadData(writeDbTbl.key)}	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#readResult
				&gt; 0,&quot;ReadData1 err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	print(&quot;ReadData1
				ok&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	for
				i = 1,#readResult do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		print(&quot;&quot;,i,(readResult[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	end </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4388_2146186683>4.11 LogPrint</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.LogPrint(LogTable)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">保存日志信息</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">待保存的内容</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">--</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">日志输出类型</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">LOG_TYPE
				= </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">{</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">ENUM_STRING
				= 0,   --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字符串类型</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   <font size="2" style="font-size: 9pt">ENUM_NUMBER
				= 1  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">数字类型
				</font></font></span></font>
				</p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">};</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ LogPrint ()</font></font></p>
				<p class="western" style="text-indent: 0.19in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">local
				LogTable = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		key
				= LOG_TYPE.ENUM_STRING,  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">日志类型</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		length
				= 0,             --value</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">数据流的总长</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		value
				= nil             -- </font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字符串或数字流</font></font></span></font></p>
				<p class="western" style="text-indent: 0.28in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">}</font></font></p>
				<p style="margin-left: 0.25in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"> <font size="2" style="font-size: 9pt">--</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">保存字符串</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	LogTable.length
				= 9		</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	LogTable.value
				= &quot;lua start&quot;</font></font></p>
				<p class="western" style="text-indent: 0.28in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">mylib.LogPrint(LogTable)</font></font></p>
				<p class="western" style="text-indent: 0.28in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">--</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">保存数据流</font></font></span></font></p>
				<p class="western" style="text-indent: 0.28in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">LogTable.key
				= LOG_TYPE.ENUM_NUMBER</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	LogTable.length
				= 20		</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	for
				i = 1,20 do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	    
				LogTable.value [i] = i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="text-indent: 0.28in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">mylib.LogPrint(LogTable)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
			</td>
		</tr>
	</table>
</center>
</div>

<h3 id=__RefHeading__4390_2146186683>4.12 GetCurTxHash</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetCurTxHash()</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取当前交易</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">hash</font></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">Hash</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">值；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetCurTxHash()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				result = {mylib.GetCurTxHash()}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">assert(#result
				== 32,&quot;GetCurTxHash err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;CurTxHash&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4392_2146186683>4.13 WriteOutput</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.WriteOutput(writeOutputTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">操作系统账户</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				writeOutputTbl = {</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	addrType
				= 1,      --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">账户类型
				</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">REG_ID
				= 0x01,BASE_58_ADDR = 0x02,</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	accountIdTbl
				= {}, --account id</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	operatorType
				= 0,   --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">操作类型</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	outHeight
				= 0,     --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">超时高度</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	moneyTbl
				= {}      --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">金额</font></font></span></font></p>
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">		}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">True</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">False</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				WriteWithDrawal(accTbl,moneyTbl)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		--</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">执行系统账户提现操作</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		assert(TableIsNotEmpty(accTbl),&quot;WriteWithDrawal
				accTbl invlaid1&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		assert(TableIsNotEmpty
				(moneyTbl),&quot;WriteWithDrawal moneyTbl invlaid1&quot;)	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		local
				writeOutputTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">			addrType
				= 1,      --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">账户类型
				</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">REG_ID
				= 0x01,BASE_58_ADDR = 0x02,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">			accountIdTbl
				= {},   --account id</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">			operatorType
				= 0,   --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">操作类型</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">			outHeight
				= 0,     --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">超时高度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">			moneyTbl
				= {}      --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">金额</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		}		</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		writeOutputTbl.accountIdTbl
				= {unpack(accTbl)}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		writeOutputTbl.operatorType
				= OPER_TYPE.</font></font> <font color="#000000"><font size="2" style="font-size: 9pt">ENUM_ADD_FREE</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		writeOutputTbl.moneyTbl
				= {unpack(moneyTbl)}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		assert(mylib.WriteOutput(writeOutputTbl),&quot;WriteWithDrawal
				WriteOutput err0&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">         <font size="2" style="font-size: 9pt">writeOutputTbl.operatorType
				= OPER_TYPE.</font></font> <font color="#000000"><font size="2" style="font-size: 9pt">ENUM_MINUS_FREE</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	   
				writeOutputTbl.accountidTbl = {mylib.GetScriptID()}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	   
				assert(mylib.WriteOutput(writeOutputTbl),&quot;WriteWithDrawal
				WriteOutput err1&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">	end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4394_2146186683>4.14 GetScriptData</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetScriptData(paraTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取脚本数据</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				paraTbl = {</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	id
				= {0x00,0x01,0x00,0x00,0xb7,0xc4}, --6</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">字节的脚本</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">id</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	
				 key = &quot;config&quot;,      --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">关键字</font></font></span></font></p>
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：脚本数据；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetScriptData()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	mylib_WriteData()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				paraTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				--id = {0x00,0x01,0x00,0x00,0xb7,0xc4}, --6</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字节的脚本</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">id</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	  id
				= {mylib.GetScriptID()},</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				key = &quot;config&quot;,      --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">关键字</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				result = {mylib.GetScriptData(paraTbl)}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#result
				&gt; 0,&quot;GetScriptData err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;GetScriptData&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4396_2146186683>4.15 GetScriptID</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetScriptID()</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取脚本</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">ID</font></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：返回</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">6</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字节的脚本</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">ID</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetScriptID()  </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				result = {mylib. GetScriptID ())}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#result
				&gt; 0,&quot; GetScriptID err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;GetScriptID&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4398_2146186683>4.16 GetCurTxAccount</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetCurTxAccount()</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取当前交易账户</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：返回账户信息；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ GetCurTxAccount ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				result = {mylib. GetCurTxAccount ())}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#result
				== 6,&quot; GetCurTxAccount err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;Account&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4400_2146186683>4.17 GetCurTxPayAmount</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetCurTxPayAmount()</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取当前交易金额</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font><font color="#000000">返回</font></span></font><font color="#000000">Int64<font face="宋体"><span lang="zh-CN">的交易金额</font><font color="#000000"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ GetCurTxPayAmount ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				paymoneyTbl = {mylib. GetCurTxPayAmount ())}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#paymoneyTbl
				== 8,&quot; GetCurTxPayAmount err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,# paymoneyTbl do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;PayAmount &quot;,i,( paymoneyTbl [i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4402_2146186683>4.18 GetUserAppAccValue</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetUserAppAccValue(idTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取指定账户的余额</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">--
				</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">用户</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">id
				</font></font></font></font>
				</p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				idTbl = {</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	idlen
				= 0,       --id</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">长度</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	idValueTbl
				= {}	 --id</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">值</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	</font></font></font></font></p>
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font><font color="#000000">返回</font></span></font><font color="#000000">Int64<font face="宋体"><span lang="zh-CN">的交易金额</font><font color="#000000"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ GetUserAppAccValue ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				idTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		idlen
				= 6,       --id</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">长度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		idValueTbl
				= {0x01,0x02,0x03,0x04,0x05,0x06}	 --id</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">值</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				money = { mylib.GetUserAppAccValue(idTbl) }</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#money
				 == 8,&quot; GetUserAppAccValue err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,# money do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;money &quot;,i,( money [i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%"></div>

<h3 id=__RefHeading__4404_2146186683>4.19 GetUserAppAccFoudWithTag</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetUserAppAccFoudWithTag(app_operateTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取指定账户的余额</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">--
				</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">用户</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">id
				</font></font></font></font>
				</p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				idTbl = {</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	idlen
				= 0,       --id</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">长度</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	idValueTbl
				= {}	 --id</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">值</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	</font></font></font></font></p>
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">	}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font><font color="#000000">返回</font></span></font><font color="#000000">Int64<font face="宋体"><span lang="zh-CN">的交易金额</font><font color="#000000"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ GetUserAppAccValue ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				idTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		idlen
				= 6,       --id</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">长度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		idValueTbl
				= {0x01,0x02,0x03,0x04,0x05,0x06}	 --id</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">值</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				money = { mylib.GetUserAppAccValue(idTbl) }</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#money
				 == 8,&quot; GetUserAppAccValue err&quot;);</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,# money do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;money &quot;,i,( money [i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">end	</font></font></p>
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4406_2146186683>4.20 WriteOutAppOperate</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.WriteOutAppOperate(app_operateTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">操作系统应用账户</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">local
				app_operateTbl = {</font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">	
				 operatorType = 0,  --</font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">操作类型</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">	
				 outHeight = 0,    --</font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">超时高度</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">	
				 moneyTbl = {},    --</font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">金额</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">	
				 userIdLen = 0,    --</font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">账户</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">ID</font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">长度</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">	
				 userIdTbl = {},   --</font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">账户</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">ID
				</font></font></font>
				</p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">	
				 fundTagLen = 0,   --fund tag len</font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">	
				 fundTagTbl = {}   --fund tag </font></font></font>
				</p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">	}</font></font></font></p>
				<p><br/>

				</p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000">True</font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">False</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ WriteOutAppOperate ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				app_operateTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		 
				operatorType = 0,  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">操作类型</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		 
				outHeight = 0,    --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">超时高度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		 
				moneyTbl = {},    --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">金额</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		 
				userIdLen = 0,    --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">账户</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">ID</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">长度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		 
				userIdTbl = {},    --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">账户</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">ID
				</font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		 
				fundTagLen = 0,   --fund tag len</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		 
				fundTagTbl = {}   --fund tag </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		}		</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	app_operateTbl.operatorType
				= AppOperatorTable.</font></font><font size="2" style="font-size: 9pt">
				</font><font color="#000000"><font size="2" style="font-size: 9pt">ENUM_ADD_FREE_OP</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	app_operateTbl.moneyTbl
				= {0,0,0,0,0,0,1,44}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	app_operateTbl.userIdLen
				= 6</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	app_operateTbl.userIdTbl
				= {0,0,0,0,2,0}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(mylib.WriteOutAppOperate(app_operateTbl),&quot;WriteOutAppOperate
				err0&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4408_2146186683>4.21 GetBase58Addr</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetBase58Addr(unpack(accountTbl))</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取指定账户的</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">base58</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">地址</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">local
				 accountTbl = {5,157,0,0,7,34}    --6</font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">字节的账户</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font size="2" style="font-size: 9pt">ID</font></font><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">
				</font></font></font></font>
				</p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000">base58<font face="宋体"><span lang="zh-CN">地址</font><font color="#000000"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetBase58Addr()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				accountTbl = {5,157,0,0,7,34}    --6</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字节的账户</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">ID</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">							</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				result = {mylib.GetBase58Addr(unpack(accountTbl))}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#result
				&gt; 0,&quot;GetBase58Addr err&quot;)	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	if(#result
				&gt; 0) then</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">		LogPrint(LOG_TYPE.NUMBER,#result,result)	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	end	</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4410_2146186683>4.22 ByteToInteger</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.
				ByteToInteger (byteTbl[1], byteTbl [2], byteTbl [3], byteTbl [4])</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">将字节转换成</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">number</font></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">4</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">字节流或</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">8</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">字节的字节流</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000">Integer<font face="宋体"><span lang="zh-CN">高度值或金额值</font><font color="#000000"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ByteToInteger()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				height =  mylib.ByteToInteger(0xa0,0x05,0x00,0x00)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(height
				&gt; 0,&quot;ByteToInteger error0&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	print(&quot;height=&quot;,height)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				money = 
				mylib.ByteToInteger(0x78,0x56,0x34,0x12,0x78,0x56,0x34,0x12)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(money
				&gt; 0,&quot;ByteToInteger error1&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	print(&quot;money=&quot;,money)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>


<h3 id=__RefHeading__4412_2146186683>4.23 IntegerToByte4</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.
				IntegerToByte4 (height)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">将</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">Interger</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">的高度值转换成</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">4</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字节</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">Integer</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">高度值</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：返回</font></font></span></font><font color="#000000">4<font face="宋体"><span lang="zh-CN">个字节</font><font color="#000000"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_IntegerToByte()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				height =  1440</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				result = {mylib.IntegerToByte4(height)}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#result
				== 4,&quot;IntegerToByte4 error0&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	print(&quot;height
				byte&quot;)	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4414_2146186683>4.24 IntegerToByte8</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.
				IntegerToByte8 (money)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">将</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">Integer</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">的高度值转换成</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">8</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字节</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">Integer</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">金额值</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：返回</font></font></span></font><font color="#000000">8<font face="宋体"><span lang="zh-CN">个字节</font><font color="#000000"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_IntegerToByte8()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				money =  1311768465173141112</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				result = {mylib.IntegerToByte8(money)}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#result
				== 8,&quot;IntegerToByte8 error0&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">for
				i = 1,#result do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	 
				print(&quot;&quot;,i,(result[i]))</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">end	</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4416_2146186683>4.25 Sha256</h3>
<div><center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.Sha256(string)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">Sha256
				hash</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">加密算法</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">待加密的内容</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">加密输出，成功返回一个</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">table</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">，否则</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_Sha256 ()</font></font></p>
				<p class="western" style="text-indent: 0.19in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				orgContent = &quot;123&quot;</font></font></p>
				<p class="western" style="text-indent: 0.19in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				content = {mylib.Sha256(orgContent)}</font></font></p>
				<p class="western" style="text-indent: 0.19in; margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	LogPrint(LOG_TYPE.ENUM_NUMBER,#content,content)</font></font></p>
				<p class="western" style="text-indent: 0.19in; margin-top: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%"></div>

<h3 id=__RefHeading__4418_2146186683>4.26 Des</h3>
<div><center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.Des(</font></font><font color="#000000"><font size="2" style="font-size: 9pt">desTbl</font></font><font color="#000000"><font size="2" style="font-size: 9pt">)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">Des</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">加密算法</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">待加密的内容</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				desTbl = </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">{</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	dataLen
				= 0, --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">加密数据长度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	data
				= {},   --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">加密数据</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	keyLen
				= 0,  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">私钥长度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	key
				= {},     --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">私钥数据</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	flag
				= 1     --1</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">加密
				</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">0</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">解密</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000">
				 <font size="2" style="font-size: 9pt">}</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">加密输出，成功返回一个</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">table</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">，否则</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_Des ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				desTbl = </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">{</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	dataLen
				= 8,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	data
				= {},</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	keyLen
				= 8,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	key
				= {},</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	flag
				= 1</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">desTbl.data
				= {0xad, 0xdd, 0x1e, 0x1b, 0xeb, 0x8c, 0x10, 0x8d}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">for
				i = 1,8 do</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	desTbl.key[i]
				= 0x33 + i</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  </font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">desTbl.flag
				= 0</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  </font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				content = {mylib.Des(desTbl)}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"> 
				<font size="2" style="font-size: 9pt">LogPrint(LOG_TYPE.ENUM_NUMBER,#content,content)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%"></div>

<h3 id=__RefHeading__4420_2146186683>4.27 VerifySignature</h3>
<div><center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.VerifySignature(sigTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">验证签名</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">待验证的内容</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				sigTbl = </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">{</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	dataLen
				=0, --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">数据长度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	data
				= {},  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">签名数据</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	keyLen
				= 0, --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">签名公钥长度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	key
				= {},   --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">签名公钥</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	hashLen
				= 0, --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">签名之前的</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">hash</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">长度</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	hash
				= {}    --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">签名</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">hash</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000">
				 <font size="2" style="font-size: 9pt">}</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">true</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">：验证成功，</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">false</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">：失败</font></font></span></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ VerifySignature ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				sigTbl = </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">{</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	dataLen
				= 9,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	data
				= {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39},</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	keyLen
				= 33,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	key
				= {0x03, 0xee, 0xf7, 0xa3, 0x80, 0xbc, 0xf9, 0xcf, 0x97, 0x5d,
				0x91, 0x6f, 0xda, 0xb1, 0x8d, 0x08, 0x1c, 0x9d, 0x55, 0xba, 0x43,
				0x46, 0x54, 0x35, 0xa4, 0xd1, 0xcc, 0x59, 0x86, 0x10, 0xa4, 0x44,
				0x79},</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	hashLen
				= 32,</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	hash
				= {0x24, 0x4f, 0xa7, 0xcf, 0x97, 0xae, 0x15, 0x85, 0xd8, 0xf8,
				0x02, 0x4b, 0xa1, 0x8b, 0x8a, 0xbe, 0xce, 0x8e, 0xb9, 0xcd, 0x4d,
				0x01, 0x6d, 0xd0, 0xba, 0x8c, 0xc0, 0xdc, 0x85, 0x1a, 0x9c, 0x0e}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  </font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				ret = mylib.VerifySignature(sigTbl)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">if
				ret then</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	LogPrint(LOG_TYPE.ENUM_STRING,string.len(&quot;ok&quot;),&quot;ok&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">else</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	LogPrint(LOG_TYPE.ENUM_STRING,string.len(&quot;bad&quot;),&quot;bad&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">end</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%"></div>

<h3 id=__RefHeading__4422_2146186683>4.28 GetTxContracts</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetTxContracts(hashTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取指定</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">hash</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">的合约内容</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">合约交易</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">hash</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：返回合约内容；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_ GetTxContracts ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				hash = {0x24, 0x4f, 0xa7, 0xcf, 0x97, 0xae, 0x15, 0x85, 0xd8,
				0xf8, 0x02, 0x4b, 0xa1, 0x8b, 0x8a, 0xbe, 0xce, 0x8e, 0xb9, 0xcd,
				0x4d, 0x01, 0x6d, 0xd0, 0xba, 0x8c, 0xc0, 0xdc, 0x85, 0x1a, 0x9c,
				0x0e}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				content = {mylib.GetTxContracts(Unpack(hash))}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"> 
				<font size="2" style="font-size: 9pt">LogPrint(LOG_TYPE.ENUM_NUMBER,#content,content)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4424_2146186683>4.29 TransferContactAsset</h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.
				TransferContactAsset (Unpack(addrTbl))</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">转移全部资产</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">账户地址</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">True</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">False</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_TransferContactAsset()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				accountTbl = {5,157,0,0,7,34}    --6</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字节的账户</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">ID</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">							</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				addrTbl = {mylib.GetBase58Addr(unpack(accountTbl))}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#addrTbl
				&gt; 0,&quot;GetBase58Addr err&quot;)	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">   
				<font size="2" style="font-size: 9pt">assert(mylib.TransferContactAsset(Unpack(addrTbl)),
				&quot;TransferContactAsset err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4426_2146186683>4.30 TransferSomeAsset</span></h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.TransferSomeAsset
				(assetOperateTbl)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">转移部分资产</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font color="#000000">
				 <font face="Times New Roman, serif"><font face="Arial, serif"><font size="2" style="font-size: 9pt">local
				assetOperateTbl = {</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font color="#000000">
				   <font face="Times New Roman, serif"><font face="Arial, serif"><font size="2" style="font-size: 9pt">toAddrTbl
				= {},  --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">转移目的地址</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font color="#000000">
				   <font face="Times New Roman, serif"><font face="Arial, serif"><font size="2" style="font-size: 9pt">outHeight
				= 0,  --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">高度
				</font></font></span></font>
				</p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font color="#000000">
				   <font face="Times New Roman, serif"><font face="Arial, serif"><font size="2" style="font-size: 9pt">moneyTbl
				= {},  --</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">资产数</font></font></span></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font color="#000000">
				   <font face="Times New Roman, serif"><font face="Arial, serif"><font size="2" style="font-size: 9pt">fundTagLen
				= 0,  --fund tag len</font></font></font></font></p>
				<p style="text-indent: 0.29in; margin-bottom: 0in"><font color="#000000">
				   <font face="Times New Roman, serif"><font face="Arial, serif"><font size="2" style="font-size: 9pt">fundTagTbl
				= {}   --fund tag </font></font></font></font>
				</p>
				<p><font color="#000000">  <font face="Times New Roman, serif"><font face="Arial, serif"><font size="2" style="font-size: 9pt">}</font></font></font></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">True</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">False</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_TransferSomeAsset()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				accountTbl = {5,157,0,0,7,34}    --6</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">字节的账户</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">ID</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">							</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	local
				addrTbl = {mylib.GetBase58Addr(unpack(accountTbl))}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	assert(#addrTbl
				&gt; 0,&quot;GetBase58Addr err&quot;)	</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">local
				assetOperateTbl = {</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">toAddrTbl
				= {}, --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">转移地址</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">	outHeight
				= 0,  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">高度
				</font></font></span></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	moneyTbl
				= {},  --</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">资产数</font></font></span></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">    <font size="2" style="font-size: 9pt">fundTagLen
				= 0,   --fund tag len</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">	fundTagTbl
				= {}   --fund tag </font></font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				money =  1311768465173141112</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				moneyTbl = {mylib.IntegerToByte8(money)}</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<br/>
<br/>

				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">assetOperateTbl.toAddrTbl
				= addrTbl</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">assetOperateTbl.outHeight
				= 1440</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">assetOperateTbl.moneyTbl
				= moneyTbl</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"> 
				<font size="2" style="font-size: 9pt">assert(mylib.TransferSomeAsset(assetOperateTbl),
				&quot;TransferSomeAsset err&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>

<h3 id=__RefHeading__4428_2146186683>4.31 GetBlockTimestamp</span></h3>
<div>
<center>
	<table width="664" cellpadding="7" cellspacing="0">
		<col width="21">
		<col width="29">
		<col width="279">
		<col width="278">
		<tr valign="top">
			<td colspan="2" width="64" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">原型</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">mylib.GetBlockTimestamp
				(height)</font></font></p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="17" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">功能</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">获取块的时间戳</font></font></span></font></p>
			</td>
		</tr>
		<tr>
			<td rowspan="2" width="21" height="26" bgcolor="#acb9ca" style="border: 1px solid #00000a; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">参数</font></b></span></font></font></font></p>
			</td>
			<td width="29" valign="top" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输入</font></b></span></font></font></font></p>
			</td>
			<td width="279" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">块的高度值，如果传入参数</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">&lt;=0</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">，则实际高度为</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">last
				block </font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">的高度
				</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">-
				</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">传入参数的绝对值；否则实际高度
				</font></font></span></font><font face="Times New Roman, serif"><font color="#000000"><font face="Arial, serif"><font size="2" style="font-size: 9pt">=
				</font></font></font></font><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">传入参数</font></font></span></font></p>
			</td>
			<td width="278" valign="top" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="29" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">输出</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">常量</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="1" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">结构体</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td rowspan="2" colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">返回</font></b></span></font></font></font></p>
			</td>
			<td width="279" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><font face="宋体"><span lang="zh-CN"><font color="#000000"><font size="2" style="font-size: 9pt">成功：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">unix</font><font face="宋体"><span lang="zh-CN"><font size="2" style="font-size: 9pt">时间戳；失败：</font></font></span></font><font color="#000000"><font size="2" style="font-size: 9pt">nil</font></font></p>
			</td>
			<td width="278" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td width="279" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
			<td width="278" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="2" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">注释</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#ffffff" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in"><br/>

				</p>
			</td>
		</tr>
		<tr valign="top">
			<td colspan="2" width="64" height="3" bgcolor="#acb9ca" style="border-top: 1px solid #00000a; border-bottom: 1px solid #00000a; border-left: 1px solid #00000a; border-right: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p align="center" style="margin-top: 0.08in; widows: 2; orphans: 2">
				<font color="#244061"><font face="宋体"><font size="3" style="font-size: 12pt"><span lang="zh-CN"><b><font size="2" style="font-size: 9pt">示例</font></b></span></font></font></font></p>
			</td>
			<td colspan="2" width="571" bgcolor="#cccccc" style="border: 1px solid #000001; padding-top: 0in; padding-bottom: 0in; padding-left: 0.08in; padding-right: 0.08in">
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"><font size="2" style="font-size: 9pt">function
				mylib_GetBlockTimestamp ()</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">local
				ts = mylib.GetBlcokTimestamp(0)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">assert(ts
				~= nil,&quot;ts is nil&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">str
				= string.format(&quot;0:%d&quot;, ts)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"> 
				<font size="2" style="font-size: 9pt">LogPrint(LOG_TYPE.ENUM_STRING,string.len(str),str)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  </font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">ts
				= mylib. GetBlcokTimestamp (100)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">assert(ts
				~= nil,&quot;ts is nil&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">str
				= string.format(&quot;100:%d&quot;, ts)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"> 
				<font size="2" style="font-size: 9pt">LogPrint(LOG_TYPE.ENUM_STRING,string.len(str),str)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  </font>
				</p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">ts
				= mylib. GetBlcokTimestamp (-100)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">assert(ts
				~= nil,&quot;ts is nil&quot;)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000">  <font size="2" style="font-size: 9pt">str
				= string.format(&quot;-100:%d&quot;, ts)</font></font></p>
				<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in">
				<font color="#000000"> 
				<font size="2" style="font-size: 9pt">LogPrint(LOG_TYPE.ENUM_STRING,string.len(str),str)</font></font></p>
				<p class="western" style="margin-top: 0.19in"><font color="#000000"><font size="2" style="font-size: 9pt">end</font></font></p>
			</td>
		</tr>
	</table>
</center>
<p class="western" style="margin-top: 0.19in; margin-bottom: 0.19in; line-height: 100%">
</div>


