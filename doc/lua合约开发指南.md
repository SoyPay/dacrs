
***<span
style="background: #f8fafe"><font size = 6>合约开发指南</font></span>***



#<span lang="zh-CN">目录</span>

##[1注意事项](#注意事项)

###[1.1由于安全性考虑，合约中只能使用lua中table，math，string库，不能使用io操作等。](#__RefHeading__1738_1702457476) 

###[1.2脚本现在只能写在一个文件中，不支持多文件。](#__RefHeading__1740_1702457476)

###[1.3脚本大小不能超过64k](#__RefHeading__1742_1702457476) 

###[1.4合约内容通过全局变量“contract”传给脚本，脚本里面可以拿来就用](#__RefHeading__1744_1702457476) 

###[1.5账户平衡检测开关通过在lua脚本中的全局变量“gCheckAccount”控制，默认关闭此开关。](#__RefHeading__1746_1702457476) 

###[1.6合约内容不能超过4096字节。](#__RefHeading__1748_1702457476)

###[1.7脚本中如果有异常，或逻辑不对，可以通过assert，error退出执行，lua解释器会捕获到该异常。](#__RefHeading__1750_1702457476)

###[1.8脚本中必须以mylib = require "mylib"开头，通过mylib调用里面的API](#__RefHeading__1752_1702457476) 

##[2示例（充值和提现操作）</span>](#__RefHeading__1754_1702457476)

##[3脚本调试</span>](#__RefHeading__1756_1702457476) 

###[3.1 启动dacrs程序</span>](#__RefHeading__1758_1702457476) 

###[3.2 导入私钥</span>](#__RefHeading__1760_1702457476) 

###[3.3 生成几个测试地址，并充值激活](#__RefHeading__1762_1702457476) 

###[3.4 注册应用脚本](#__RefHeading__1764_1702457476) 
###[3.5充值操作](#__RefHeading__1766_1702457476) 

###[3.6 提现操作](#__RefHeading__1768_1702457476) 

####[3.6.1查看dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6当前余额：](#__RefHeading__1770_1702457476)

####[3.6.2 提现](#__RefHeading__1772_1702457476)

####[3.6.3再次查看dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6当前余额，发现已经提现了5个币。](#__RefHeading__1774_1702457476) 

###[3.7 脚本出错处理](#__RefHeading__1776_1702457476) 

##[4部署](#__RefHeading__1778_1702457476) 

<br/>

**开发规范请参考《</span>lua<span
lang="zh-CN">脚本规范</span>.txt<span lang="zh-CN">》</span>**

**RPC<span lang="zh-CN">接口请参考《</span>RPCCommandV0.1 beta.docx<span
lang="zh-CN">》</span>**

**<span lang="zh-CN">原理请参考《</span>DACRS<span
lang="zh-CN">系统智能合约</span>LUA<span
lang="zh-CN">应用开发原理</span>.docx<span lang="zh-CN">》</span>**

**LUA SDK API<span lang="zh-CN">请参考《智能合约</span>API—LUA.docx<span
lang="zh-CN">》</span>**
</br>


<h2 id="注意事项">1 注意事项</h>

<h3 id=__RefHeading__1738_1702457476> 1.1由于安全性考虑，合约中只能使用lua中table，math，tring库，不能使用io操作等. </h2>

<h3 id=__RefHeading__1740_1702457476> 1.2<span lang="zh-CN">脚本现在只能写在一个文件中，不支持多文件。</h2>

<h3 id=__RefHeading__1742_1702457476> 1.3<span lang="zh-CN">脚本大小不能超过</span>64k<span lang="zh-CN">。</h2>

<h3 id=__RefHeading__1744_1702457476> 1.4<span lang="zh-CN">合约内容通过全局变量“</span>contract”<span lang="zh-CN">传给脚本，脚本里面可以拿来就用</span> </h2>

<h3 id=__RefHeading__1746_1702457476> 1.5<span lang="zh-CN">账户平衡检测开关通过在</span>lua<span lang="zh-CN">脚本中的全局变量“</span>gCheckAccount”<span lang="zh-CN">控制，默认关闭此开关。</span> </h2>

<h3 id=__RefHeading__1748_1702457476> 1.6<span lang="zh-CN">合约内容不能超过</span>4096<span lang="zh-CN">字节。</span> </h2>

<h3 id=__RefHeading__1750_1702457476> 1.7<span lang="zh-CN">脚本中如果有异常，或逻辑不对，可以通过</span>assert<span lang="zh-CN">，</span>error<span lang="zh-CN">退出执行，</span>lua<span lang="zh-CN">解释器会捕获到该异常。</span> </h2>

<h3 id=__RefHeading__1752_1702457476> 1.8<span lang="zh-CN">脚本中必须以</span>mylib = require "mylib"<span lang="zh-CN">开头，通过</span>mylib<span lang="zh-CN">调用里面的</span>API<span lang="zh-CN">。</span> </h2>


<h2 id=__RefHeading__1754_1702457476>2 示例（充值和提现操作）</h2>

<p style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN">该示例所使用的</span></font><font face="宋体, serif">SDK
API</font><font face="宋体"><span lang="zh-CN">，请参考《智能合约</span></font><font face="宋体, serif">API—LUA.docx</font><font face="宋体"><span lang="zh-CN">》，文档里有详细的例子。</span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">mylib
= require &quot;mylib&quot;</font></p>


<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">日志类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">LOG_TYPE
= </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">{</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  
<font face="华文楷体, serif">ENUM_STRING = 0, --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字符串类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  
<font face="华文楷体, serif">ENUM_NUMBER = 1, --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">数字类型
</font></span></font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">};</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">系统账户操作定义</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">OPER_TYPE
= </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">{</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	ENUM_ADD_FREE
= 1,   --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">系统账户加</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	ENUM_MINUS_FREE
= 2  --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">系统账户减</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">脚本应用账户操作类型定义</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">APP_OPERATOR_TYPE
= </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">{
</font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	ENUM_ADD_FREE_OP
= 1,      --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">自由账户加</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	ENUM_SUB_FREE_OP
= 2,      --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">自由账户减</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	ENUM_ADD_FREEZED_OP
= 3,   --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">冻结账户加</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	ENUM_SUB_FREEZED_OP
= 4    --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">冻结账户减</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">账户类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">ADDR_TYPE
= </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">{</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	ENUM_REGID
= 1,		-- REG_ID</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	ENUM_BASE58
= 2,	-- BASE58 ADDR</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">交易类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">TX_TYPE
= </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">{</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	TX_WITHDRAW
= 1,	--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">提现</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	TX_RECHARGE=
2, 	--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值</font></span></font><font face="华文楷体, serif">	
</font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">FREEZE_MONTH_NUM
= 20   	-- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">冻结次数</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">FREEZE_PERIOD
= 5	-- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">冻结周期</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">MAX_MONEY
= 100000000000000000 -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">总金额限制</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">FREE_MONEY
= 10000000000000000 -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">自由金额限制</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">gCheckAccount
= true -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">平衡检查
</font></span></font><font face="华文楷体, serif">true </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">打开，</font></span></font><font face="华文楷体, serif">false
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">关闭</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">gSendAccountTbl
= 			-- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">发币地址</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">{</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	0x00,
0x00, 0x00, 0x00, 0x01, 0x00</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">判断表是否为空</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
TableIsEmpty(t)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	return
_G.next(t) == nil</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">判断表非空</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
TableIsNotEmpty(t)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">return false == TableIsEmpty(t)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--[[</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="宋体"><span lang="zh-CN"><font face="华文楷体">功能</font></span></font><font face="华文楷体, serif">:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">日志输出</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">参数：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	aKey:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">日志类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	bLength:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">日志长度</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	cValue</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">：日志</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--]]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
LogPrint(aKey,bLength,cValue)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(bLength
&gt;= 1,&quot;LogPrint bLength invlaid&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	if(aKey
== LOG_TYPE.ENUM_STRING) then  </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">     
<font face="华文楷体, serif">assert(type(cValue) ==
&quot;string&quot;,&quot;LogPrint cValue invlaid0&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	elseif(aKey
== LOG_TYPE.ENUM_NUMBER)	 then	   </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	
   assert(TableIsNotEmpty(cValue),&quot;LogPrint cValue invlaid1&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	else</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	
   error(&quot;LogPrint aKey invlaid&quot;) </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
logTable = {</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		key
= LOG_TYPE.ENUM_STRING,  </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		length
= 0,             </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		value
= nil             </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">    <font face="华文楷体, serif">}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	logTable.key
= aKey </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	logTable.length
= bLength		</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	logTable.value
= cValue</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	mylib.LogPrint(logTable)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--[[</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="宋体"><span lang="zh-CN"><font face="华文楷体">功能</font></span></font><font face="华文楷体, serif">:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">遍历表元素</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">参数：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	t:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">表</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	i:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">开始索引</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--]]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
Unpack(t,i)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   <font face="华文楷体, serif">i
= i or 1</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   <font face="华文楷体, serif">if
t[i] then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">    
<font face="华文楷体, serif">return t[i],Unpack(t,i+1)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   <font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--[[</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="宋体"><span lang="zh-CN"><font face="华文楷体">功能</font></span></font><font face="华文楷体, serif">:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">比较两表元素是否相同</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">参数：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	tDest:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">表</font></span></font><font face="华文楷体, serif">1</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	tSrc:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">表</font></span></font><font face="华文楷体, serif">2</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	start1:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">开始索引</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">返回值：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	0:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">相等</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	1:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">表</font></span></font><font face="华文楷体, serif">1&gt;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">表</font></span></font><font face="华文楷体, serif">2</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	2:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">表</font></span></font><font face="华文楷体, serif">1&lt;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">表</font></span></font><font face="华文楷体, serif">2</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--]]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
MemCmp(tDest,tSrc,start1)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(TableIsNotEmpty(tDest),&quot;tDest
is empty&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(TableIsNotEmpty(tSrc),&quot;tSrc
is empty&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
i </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	for
i = #tDest,1,-1 do </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		if
tDest[i] &gt; tSrc[i + start1 - 1] then </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">			return
1  </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		elseif
	tDest[i] &lt; tSrc[i + start1 - 1] then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">			return
-1 </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		else
</font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	return
0 </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--[[</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="宋体"><span lang="zh-CN"><font face="华文楷体">功能</font></span></font><font face="华文楷体, serif">:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">获取表从开始索引指定长度的元素集合</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">参数：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	tbl:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">表</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	start:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">开始索引</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	length:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">长度</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">返回值：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">一个新表</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--]]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
GetValueFromContract(tbl,start,length)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="华文楷体, serif">assert(start &gt;
0,&quot;GetValueFromContract start err&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="华文楷体, serif">assert(length &gt;
0,&quot;GetValueFromContract length err&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">local
newTab = {}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
i</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	for
i = 0,length -1 do</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		newTab[1
+ i] = tbl[start + i]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">return newTab</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--[[</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">功能</font></span></font><font face="华文楷体, serif">:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">参数：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">无</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">返回值：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	true:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">成功</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	false:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">失败</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">流程：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	1</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）获取当前交易账户，并判断是否与发币地址一致</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	2</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）获取合约相应内容，并判断合法性</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	3</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）将自由金额提现到系统账户下</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	4</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）冻结相应的冻结部分</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	5</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）扣除相应的脚本账户下的金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--]]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
Recharge()</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
1</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
local accountTbl = {0, 0, 0, 0, 0, 0}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
accountTbl = {mylib.GetCurTxAccount()}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
assert(TableIsNotEmpty(accountTbl),&quot;GetCurTxAccount error&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
assert(MemCmp(gSendAccountTbl, accountTbl,1) == 0,&quot;Recharge
address err&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
2</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
toAddrTbl = {}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	toAddrTbl
= GetValueFromContract(contract, 3, 34)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
moneyTbl = {}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	moneyTbl
= GetValueFromContract(contract, 37, 8)  </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">local money =
mylib.ByteToInteger(Unpack(moneyTbl))</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(money
&gt; 0,&quot;money &lt;= 0&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
freeMoneyTbl = {}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	freeMoneyTbl
= GetValueFromContract(contract, 45, 8)  </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">local freeMoney =
mylib.ByteToInteger(Unpack(freeMoneyTbl))</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(freeMoney
&gt; 0,&quot;freeMoney &lt;= 0&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
freeMonthMoneyTbl = {}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	freeMonthMoneyTbl
= GetValueFromContract(contract, 53, 8)  </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">local freeMonthMoney =
mylib.ByteToInteger(Unpack(freeMonthMoneyTbl))</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(freeMonthMoney
&gt; 0,&quot;freeMonthMoney &lt;= 0&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
payMoneyTbl = {}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	payMoneyTbl
= {mylib.GetCurTxPayAmount()}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(TableIsNotEmpty(payMoneyTbl),&quot;GetCurTxPayAmount
error&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
payMoney = mylib.ByteToInteger(Unpack(payMoneyTbl))</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(payMoney
&gt; 0,&quot;payMoney &lt;= 0&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">总金额与充值金额要相等</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(money
== payMoney, &quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">1&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">总金额不能小于</font></span></font><font face="华文楷体, serif">1,</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">不能小于自由金额或月冻结金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	if
money &lt; 1 </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		or
money &lt; freeMoney </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		or
money &lt; freeMonthMoney then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		LogPrint(LOG_TYPE.ENUM_STRING,
string.len(&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">2&quot;),
&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">2&quot;);</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		error(&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">2&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">三个金额，上限值检测</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	if
money &gt;= MAX_MONEY </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		or
freeMoney &gt;= FREE_MONEY</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		or
freeMonthMoney &gt;= FREE_MONEY then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		LogPrint(LOG_TYPE.ENUM_STRING,
string.len(&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">3&quot;),
&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">3&quot;);</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		error(&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">3&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
freezeNum = FREEZE_MONTH_NUM - 1</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(freezeNum
&gt; 0, &quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">月冻结总数不正确</font></span></font><font face="华文楷体, serif">&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">总金额不能小于总的月冻结金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
freezeMoney = freeMonthMoney * freezeNum</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	if
 freezeMoney &lt; freezeNum</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		or
freezeMoney &lt; freeMoney </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		or
money &lt; freezeMoney then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		LogPrint(LOG_TYPE.ENUM_STRING,
string.len(&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">4&quot;),
&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">4&quot;);</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		error(&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">4&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">检查充值总金额和自由金额加上冻结金额是否相等</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	freezeMoney
= freezeMoney + freeMoney</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	if
money ~= freezeMoney then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		LogPrint(LOG_TYPE.ENUM_STRING,
string.len(&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">5&quot;),
&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">5&quot;);</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		error(&quot;</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值金额不正确</font></span></font><font face="华文楷体, serif">5&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
3</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作系统账户的结构</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
writeOutputTbl = </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	{</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		addrType
= 1,       --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">账户类型
</font></span></font><font face="华文楷体, serif">REG_ID =
0x01,BASE_58_ADDR = 0x02,</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		accountIdTbl
= {},  --account id</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		operatorType
= 0,   --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		outHeight
= 0,      --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">超时高度</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		moneyTbl
= {}       --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.addrType
= ADDR_TYPE.ENUM_BASE58</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.operatorType
= OPER_TYPE.ENUM_ADD_FREE</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.accountIdTbl
= {Unpack(toAddrTbl)}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.moneyTbl
= {Unpack(freeMoneyTbl)}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(mylib.WriteOutput(writeOutputTbl),&quot;WriteOutput
err1&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
curHeight = 0</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">curHeight =
mylib.GetCurRunEnvHeight()</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
4</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
appOperateTbl = {</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		operatorType
= 0, -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		outHeight
= 0,    -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">超时高度</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		moneyTbl
= {},    </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		userIdLen
= 0,    -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">地址长度</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		userIdTbl
= {},   -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">地址</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		fundTagLen
= 0,   -- fund tag len</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		fundTagTbl
= {}   -- fund tag </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	}
</font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	appOperateTbl.operatorType
= APP_OPERATOR_TYPE.ENUM_ADD_FREEZED_OP</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">appOperateTbl.userIdLen = 34</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">appOperateTbl.userIdTbl = toAddrTbl</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">appOperateTbl.moneyTbl =
freeMonthMoneyTbl</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	for
i = 1, freezeNum do </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">     
<font face="华文楷体, serif">appOperateTbl.outHeight = curHeight
+ FREEZE_PERIOD * i</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">     
<font face="华文楷体, serif">assert(mylib.WriteOutAppOperate(appOperateTbl),&quot;WriteOutAppOperate
err1&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">    <font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
5</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.addrType
= ADDR_TYPE.ENUM_REGID</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.operatorType
= OPER_TYPE.ENUM_MINUS_FREE</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.accountIdTbl
= {mylib.GetScriptID()}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(mylib.WriteOutput(writeOutputTbl),&quot;WriteOutput
err2&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	return
true</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--[[</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="宋体"><span lang="zh-CN"><font face="华文楷体">功能</font></span></font><font face="华文楷体, serif">:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作系统账户</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">参数：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	accTbl:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">账户</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	moneyTbl:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">返回值：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	true:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">成功</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	false:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">失败</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">流程：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	1</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）增加该系统账户金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	2</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）相应地减少脚本账户金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--]]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
WriteWithdrawal(accTbl,moneyTbl)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作系统账户的结构</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
writeOutputTbl = </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	{</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		addrType
= 1,       --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">账户类型
</font></span></font><font face="华文楷体, serif">REG_ID =
0x01,BASE_58_ADDR = 0x02,</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		accountIdTbl
= {},  --account id</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		operatorType
= 0,   --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		outHeight
= 0,      --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">超时高度</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		moneyTbl
= {}       --</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	}	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">执行系统账户提现操作</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(TableIsNotEmpty(accTbl),&quot;WriteWithDrawal
accTbl invlaid1&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(TableIsNotEmpty(moneyTbl),&quot;WriteWithDrawal
moneyTbl invlaid1&quot;)	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
1</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.addrType
= ADDR_TYPE.ENUM_REGID</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.operatorType
= OPER_TYPE.ENUM_ADD_FREE</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.accountIdTbl
= {Unpack(accTbl)}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.moneyTbl
= {Unpack(moneyTbl)}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(mylib.WriteOutput(writeOutputTbl),&quot;WriteWithDrawal
WriteOutput err0&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
2</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.operatorType
= OPER_TYPE.ENUM_MINUS_FREE</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	writeOutputTbl.accountIdTbl
= {mylib.GetScriptID()}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(mylib.WriteOutput(writeOutputTbl),&quot;WriteWithDrawal
WriteOutput err1&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	return
true</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--[[</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">功能</font></span></font><font face="华文楷体, serif">:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">提现</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">参数：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	addrType:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">账户类型，</font></span></font><font face="华文楷体, serif">BASE58</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">或</font></span></font><font face="华文楷体, serif">REGID</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">返回值：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	true:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">成功</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	false:</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">失败</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="宋体"><span lang="zh-CN"><font face="华文楷体">流程：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	1</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）判断账户类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	2</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）获取当前交易账户</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	3</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）获取该账户下的自由金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	4</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）减掉该账户下的自由金额</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	5</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）相应的操作系统账户</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">--]]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
Withdraw(addrType)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
1</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	if
addrType ~= ADDR_TYPE.ENUM_REGID and addrType ~=
ADDR_TYPE.ENUM_BASE58 then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		error(&quot;In
function Withdraw, addr type err&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
2</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
accountTbl = {0, 0, 0, 0, 0, 0}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	accountTbl
= {mylib.GetCurTxAccount()}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(TableIsNotEmpty(accountTbl),&quot;GetCurTxAccount
error&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
idTbl = </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	{</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		idLen
= 0,       </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		idValueTbl
= {}	 </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	if
addrType == ADDR_TYPE.ENUM_REGID then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		idTbl.idLen
= 6</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		idTbl.idValueTbl
= accountTbl</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	else</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		local
base58Addr = {}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		base58Addr
= {mylib.GetBase58Addr(Unpack(accountTbl))}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		assert(TableIsNotEmpty(base58Addr),&quot;GetBase58Addr
error&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		idTbl.idLen
= 34</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		idTbl.idValueTbl
= base58Addr</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
3</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
freeMoneyTbl = {mylib.GetUserAppAccValue(idTbl)}</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(TableIsNotEmpty(freeMoneyTbl),&quot;GetUserAppAccValue
error&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
freeMoney = mylib.ByteToInteger(Unpack(freeMoneyTbl))</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(freeMoney
&gt; 0,&quot;Account balance is 0.&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
4</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	local
appOperateTbl = {</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		operatorType
= 0, -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作类型</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		outHeight
= 0,    -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">超时高度</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		moneyTbl
= {},    </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		userIdLen
= 0,    -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">地址长度</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		userIdTbl
= {},   -- </font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">地址</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		fundTagLen
= 0,   -- fund tag len</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">		fundTagTbl
= {}   -- fund tag </font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	}
</font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">appOperateTbl.operatorType =
APP_OPERATOR_TYPE.ENUM_SUB_FREE_OP</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">appOperateTbl.userIdLen =
idTbl.idLen</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">appOperateTbl.userIdTbl =
idTbl.idValueTbl</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">appOperateTbl.moneyTbl =
freeMoneyTbl</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">assert(mylib.WriteOutAppOperate(appOperateTbl),&quot;WriteOutAppOperate
err1&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	--
5</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	assert(WriteWithdrawal(accountTbl,
freeMoneyTbl), &quot;WriteWithdrawal err&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">	return
true</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">function
Main()</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">--[[</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">local
i = 1</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">for
i = 1,#contract do</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">print(&quot;contract&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">print(&quot;  &quot;,i,(contract[i]))	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">--]]</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="华文楷体, serif">assert(#contract &gt;= 2,&quot;contract
length err.&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="华文楷体, serif">assert(contract[1] == 0xff,&quot;Contract
identification error.&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">if
contract[2] == TX_TYPE.TX_RECHARGE then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">assert(#contract == 60,&quot;recharge
contract length err.&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">Recharge()	</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"> 
<font face="华文楷体, serif">elseif contract[2] == 
TX_TYPE.TX_WITHDRAW then</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">assert(#contract == 11,&quot;withdraw
contract length err.&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">Withdraw(contract[3])</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">else</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="华文楷体, serif">error(&quot;RUN_SCRIPT_DATA_ERR&quot;)</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">  <font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">end</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">Main()</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>

<br/>
<h2 id=__RefHeading__1756_1702457476>3脚本调试</h2>  

<span
lang="zh-CN">以上面的示例代码为例，将其代码保存为</span>test.lua<span
lang="zh-CN">，文件格式保存为</span>utf-8<span lang="zh-CN">：</span>

<h3 id=__RefHeading__1758_1702457476> 3.1 <span lang="zh-CN">启动</span>dacrs<span lang="zh-CN">程序</span></h3>

<span lang="zh-CN">最好在</span>linux<span
lang="zh-CN">下测试，</span>windows<span
lang="zh-CN">上编译，调试慢。调试时先在</span>regtest<span
lang="zh-CN">下调试通过，然后才在</span>testnet<span
lang="zh-CN">和</span>main<span lang="zh-CN">网络上注册。</span>

<span lang="zh-CN">修改</span>Dacrs.conf<span
lang="zh-CN">文件，添加</span>regtest=1<span
lang="zh-CN">，打开</span>vm<span
lang="zh-CN">调试开关，</span>debug=vm<span
lang="zh-CN">，注释掉</span>gen=1<span
lang="zh-CN">，</span>genproclimit=1000000<span
lang="zh-CN">，采用手动挖矿。</span>

<div>
rpcuser=smartcoin<br/>
rpcpassword=admin<br/>
blockminsize=1000<br/>
debug=ERROR<br/>
#debug=vm<br/>
#debug=NOUI<br/>
#debug=TOUI<br/>
debug=INFO<br/>
#debug=net<br/>
#debug=sendtx<br/>
printtoconsole=0<br/>
logtimestamps=1<br/>
logprintfofile=1<br/>
logprintfileline=1<br/>
listen=1<br/>
server=1<br/>
rpcport=18332<br/>
uiport=4246<br/>
#gen=1<br/>
genproclimit=1000000<br/>
logmaxsize=100<br/>
regtest=1<br/>
</div>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>



<span lang="zh-CN">下面所用到的</span>RPC<span
lang="zh-CN">命令，请参考</span>RPC<span
lang="zh-CN">接口文档《</span>RPCCommandV0.1 beta.docx<span
lang="zh-CN">》。</span>

<br/>

<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4">Dacrs
version v1.1.2.5-8a88c65-dirty-release-linux (2016-10-11 10:03:03
+0800)</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4">Using
OpenSSL version OpenSSL 1.0.1f 6 Jan 2014</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4">Startup
time: 2016-10-11 08:14:26</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4">Default
data directory /home/xgc/.Dacrs</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4">Using
data directory /home/share/xgc/dacrs_test/regtest</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4">Using
at most 125 connections (		1024 file descriptors available)</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>

<br/>

<h3 id=__RefHeading__1760_1702457476> 3.2 <span lang="zh-CN">导入私钥</span> </h3>

<br/>

<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test importprivkey
cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;imorpt
key address&quot; : &quot;dk2NNjraSvquD9b4SQbysVRQeFikA55HLi&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getaccountinfo
dk2NNjraSvquD9b4SQbysVRQeFikA55HLi</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;Address&quot;
: &quot;dk2NNjraSvquD9b4SQbysVRQeFikA55HLi&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;KeyID&quot;
: &quot;4fb64dd1d825bb6812a7090a1d0dd2c75b55242e&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;RegID&quot;
: &quot;0-20&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;PublicKey&quot;
:
&quot;03ae28a4100145a4c354338c727a54800dc540069fa2f5fd5d4a1c80b4a35a1762&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;MinerPKey&quot;
: &quot;&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;Balance&quot;
: 100000082000000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;CoinDays&quot;
: 0,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;UpdateHeight&quot;
: 263,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;CurCoinDays&quot;
: 694445,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;postion&quot;
: &quot;inblock&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>



<h3 id=__RefHeading__1762_1702457476> 3.3 <span lang="zh-CN">生成几个测试地址，并充值激活</span> </h3>


<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4" >xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getnewaddress</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
    <font face="宋体, serif"><font size="4" >&quot;addr&quot;
: &quot;druMpuM8YEkvzyErLMUgwasyafoficmK7S&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
    <font face="宋体, serif"><font size="4" >&quot;minerpubkey&quot;
: &quot;no&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4" >xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getnewaddress</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
    <font face="宋体, serif"><font size="4" ">&quot;addr&quot;
: &quot;dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
    <font face="宋体, serif"><font size="4" >&quot;minerpubkey&quot;
: &quot;no&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%; widows: 2; orphans: 2">
<font face="宋体"><span lang="zh-CN"><font face="宋体"><font size="4" >充币：</font></font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test sendtoaddress
druMpuM8YEkvzyErLMUgwasyafoficmK7S 100000000000</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;hash&quot;
: &quot;59b55b1abe2a6aa38d85301158d486ffa367f349af9c18506a4adadc1d999a41&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test sendtoaddress
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6 100000000000</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;hash&quot;
: &quot;e6ac958bbc202cb300bbfcb21ed66e6279f009128ec17e9354796e3d12ae5dd5&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font size="4">手动挖矿，看生成的两地址里现在是否有币</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test setgenerate true 1</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;msg&quot;
: &quot;in  mining&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getbalance
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;balance&quot;
: 1000.00000000</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getbalance
druMpuM8YEkvzyErLMUgwasyafoficmK7S </font></font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;balance&quot;
: 1000.00000000</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font size="4" >激活这两个地址</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test registaccounttx
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6 100000</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;hash&quot;
: &quot;02d86cdecb3cb9a9d92bd28cd6023c58f771bd3df03aa0689e91b26c2307a976&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test registaccounttx
druMpuM8YEkvzyErLMUgwasyafoficmK7S 100000</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;hash&quot;
: &quot;6638af4edc5e417d8abd214bb6c1db1f41f83ea2e4b8c5924d2bcebfe4e25741&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test setgenerate true 1</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;msg&quot;
: &quot;in  mining&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getaccountinfo
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;Address&quot;
: &quot;dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;KeyID&quot;
: &quot;8dfa4bec33ea7ce2692a6dc4ea3e47e1085cc0b7&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;RegID&quot;
: &quot;293-1&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4">&quot;PublicKey&quot;
:
&quot;0208f52f6e296805bb8aa44a3c44454cc15ed5abca43b9f95ae73f1ef2c99f2bc1&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;MinerPKey&quot;
: &quot;&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;Balance&quot;
: 99999900000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;CoinDays&quot;
: 9,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;UpdateHeight&quot;
: 293,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;CurCoinDays&quot;
: 9,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;postion&quot;
: &quot;inblock&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getaccountinfo
druMpuM8YEkvzyErLMUgwasyafoficmK7S</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;Address&quot;
: &quot;druMpuM8YEkvzyErLMUgwasyafoficmK7S&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;KeyID&quot;
: &quot;9b2bd08576843e5416c7a90739b8ea50d4e98b4f&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;RegID&quot;
: &quot;293-2&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;PublicKey&quot;
:
&quot;027826fbe5ce964c4bd1d55217d103f21214ef603da24224b9d4eee182a2353d32&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;MinerPKey&quot;
: &quot;&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;Balance&quot;
: 99999900000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;CoinDays&quot;
: 9,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;UpdateHeight&quot;
: 293,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;CurCoinDays&quot;
: 9,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;postion&quot;
: &quot;inblock&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">RegID</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >字段分别为</font></span></font><font face="宋体, serif"><font size="4" 293-1,293-2,</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >表示地址已经激活。</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>

<br/>

<h3 id=__RefHeading__1764_1702457476>  3.4 <span lang="zh-CN">注册应用脚本</span> </h3>

<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4"> xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test registerapptx 
druMpuM8YEkvzyErLMUgwasyafoficmK7S
/home/share/xgc/dacrs_test/data/test.lua 110000000  0 &quot;test&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;hash&quot;
: &quot;b9e8906135e64eec879a0dc38079031346dccb0ba273b5503dcd7f3bb0e42e6f&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test setgenerate true 1</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;msg&quot;
: &quot;in  mining&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getscriptid
b9e8906135e64eec879a0dc38079031346dccb0ba273b5503dcd7f3bb0e42e6f</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;regid:&quot;
: &quot;295-1&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;script&quot;
: &quot;270100000100&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font size="4" >获取到脚本</font></span></font><font face="宋体, serif"><font size="4" >appid</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >为</font></span></font><font face="宋体, serif"><font size="4" >295-1</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>

<br/>

<h3 id=__RefHeading__1766_1702457476> 3.5 <span lang="zh-CN">充值操作</span> </h3>


<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">合约内容</font></span></font><font face="华文楷体, serif">=</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">前缀</font></span></font><font face="华文楷体, serif">1</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节（</font></span></font><font face="华文楷体, serif">0xff</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">+
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作类型</font></span></font><font face="华文楷体, serif">1</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节（</font></span></font><font face="华文楷体, serif">0x02</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">+
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值地址</font></span></font><font face="华文楷体, serif">34</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节（</font></span></font><font face="华文楷体, serif">dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">+
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值总金额
</font></span></font><font face="华文楷体, serif">8</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节（</font></span></font><font face="华文楷体, serif">10000000000</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">+
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">自由金额
</font></span></font><font face="华文楷体, serif">8</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节（</font></span></font><font face="华文楷体, serif">500000000</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">+
</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">每月冻结金额
</font></span></font><font face="华文楷体, serif">8</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节（</font></span></font><font face="华文楷体, serif">500000000</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">将合约内容转换成</font></span></font><font face="华文楷体, serif">16</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">进制字符串</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">前缀（</font></span></font><font face="华文楷体, serif">0xff</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">=&gt;
ff</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">操作类型（</font></span></font><font face="华文楷体, serif">0x02</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">=&gt;
02</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值地址（</font></span></font><font face="华文楷体, serif">dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">=&gt;</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">d</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">的</font></span></font><font face="华文楷体, serif">ascii</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">码</font></span></font><font face="华文楷体, serif">10</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">进制是</font></span></font><font face="华文楷体, serif">100</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">，转成</font></span></font><font face="华文楷体, serif">16</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">进制是</font></span></font><font face="华文楷体, serif">64</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">q</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">的</font></span></font><font face="华文楷体, serif">ascii</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">码</font></span></font><font face="华文楷体, serif">10</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">进制是</font></span></font><font face="华文楷体, serif">113</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">，转成</font></span></font><font face="华文楷体, serif">16</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">进制是</font></span></font><font face="华文楷体, serif">71</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">依次转换，最终结果为</font></span></font><font face="华文楷体, serif">6471686269716a705436376f555832615458753366416362707670634a4556345936</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">充值总金额（</font></span></font><font face="华文楷体, serif">10000000000</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">=&gt;
</font>
</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">利用计算器转成</font></span></font><font face="华文楷体, serif">16</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">进制为</font></span></font><font face="华文楷体, serif">2540be400</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">，补齐</font></span></font><font face="华文楷体, serif">8</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节后为</font></span></font><font face="华文楷体, serif">00
00 00 02 54 0b e4 00</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">，按照内存中逆序后为</font></span></font><font face="华文楷体, serif">00e40b5402000000</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">自由金额（</font></span></font><font face="华文楷体, serif">500000000</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">=&gt;</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">利用计算器转成</font></span></font><font face="华文楷体, serif">16</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">进制为</font></span></font><font face="华文楷体, serif">1dcd6500</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">，补齐</font></span></font><font face="华文楷体, serif">8</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节后为</font></span></font><font face="华文楷体, serif">00
00 00 00 1d cd 65 00</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">，按照内存中逆序后为</font></span></font><font face="华文楷体, serif">0065cd1d00000000</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">每月冻结金额（</font></span></font><font face="华文楷体, serif">500000000</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">）</font></span></font><font face="华文楷体, serif">=&gt;</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">利用计算器转成</font></span></font><font face="华文楷体, serif">16</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">进制为</font></span></font><font face="华文楷体, serif">1dcd6500</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">，补齐</font></span></font><font face="华文楷体, serif">8</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">字节后为</font></span></font><font face="华文楷体, serif">00
00 00 00 1d cd 65 00</font><font face="宋体"><span lang="zh-CN"><font face="华文楷体">，按照内存中逆序后为</font></span></font><font face="华文楷体, serif">0065cd1d00000000</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font face="华文楷体">将这些字段组合在一起，形成合约内容：</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="华文楷体, serif">ff026471686269716a705436376f555832615458753366416362707670634a455634593600e40b54020000000065cd1d000000000065cd1d00000000</font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test createcontracttx
druMpuM8YEkvzyErLMUgwasyafoficmK7S 295-1 10000000000
ff026471686269716a705436376f555832615458753366416362707670634a455634593600e40b54020000000065cd1d000000000065cd1d00000000
100000 0</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;hash&quot;
: &quot;fadb05dca69b2a9a5ee771ec6533b2b856950868af8589476e920c2eea5a465c&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test setgenerate true 1</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;msg&quot;
: &quot;in  mining&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getappaccinfo 295-1
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;mAccUserID&quot;
:
&quot;6471686269716a705436376f555832615458753366416362707670634a4556345936&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;FreeValues&quot;
: 0,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;vFreezedFund&quot;
: [</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 309,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 314,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 319,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 324,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 329,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 334,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 339,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 344,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 349,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 354,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 359,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4">},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 364,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" },</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 369,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 374,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 379,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 384,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4">&quot;nHeight&quot;
: 389,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4">&quot;nHeight&quot;
: 394,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 399,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">    <font face="宋体, serif"><font size="4" >]</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font size="4" >手动多挖几次矿，等自由金额不为</font></span></font><font face="宋体, serif"><font size="4" >0</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >时，可以提现操作</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test getappaccinfo 295-1
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;mAccUserID&quot;
:
&quot;6471686269716a705436376f555832615458753366416362707670634a4556345936&quot;,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;FreeValues&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">   
<font face="宋体, serif"><font size="4" >&quot;vFreezedFund&quot;
: [</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 314,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 319,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 324,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 329,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4">&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 334,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 339,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4">{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 344,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 349,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 354,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="1" style="font-size: 7pt">&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4">},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 359,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 364,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4">&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 369,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4">&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 374,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 379,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 384,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4">&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4">},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 389,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 394,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >},</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >{</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;value&quot;
: 500000000,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;nHeight&quot;
: 399,</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
    <font face="宋体, serif"><font size="4" >&quot;vTag&quot;
: &quot;&quot;</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">       
<font face="宋体, serif"><font size="4" >}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">    <font face="宋体, serif"><font size="4" >]</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >}</font></font></p>

<h3 id=__RefHeading__1768_1702457476> 3.6 <span lang="zh-CN">提现操作</span> </h3>

<span lang="zh-CN">将自由金额（</span>5<span
lang="zh-CN">个币），提现到地址</span>dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6



<span lang="zh-CN">合约内容</span>=<span lang="zh-CN">前缀</span>1<span
lang="zh-CN">字节（</span>0xff<span lang="zh-CN">）</span>+ <span
lang="zh-CN">操作类型</span>1<span lang="zh-CN">字节（</span>0x01<span
lang="zh-CN">）</span>+ <span lang="zh-CN">账户类型</span>1<span
lang="zh-CN">字节（</span>0x02<span lang="zh-CN">）</span>+ <span
lang="zh-CN">提现金额 </span>8<span
lang="zh-CN">字节（</span>500000000<span lang="zh-CN">）</span>



<span lang="zh-CN">将合约内容转换成</span>16<span
lang="zh-CN">进制字符串</span>

<span lang="zh-CN">前缀（</span>0xff<span lang="zh-CN">）</span>=&gt; ff

<span lang="zh-CN">操作类型（</span>0x02<span
lang="zh-CN">）</span>=&gt; 01

<span lang="zh-CN">账户类型（</span>0x02<span
lang="zh-CN">）</span>=&gt; 02

<span lang="zh-CN">提现金额（</span>500000000<span
lang="zh-CN">）</span>=&gt;

<span lang="zh-CN">利用计算器转成</span>16<span
lang="zh-CN">进制为</span>1dcd6500<span lang="zh-CN">，补齐</span>8<span
lang="zh-CN">字节后为</span>00 00 00 00 1d cd 65 00<span
lang="zh-CN">，按照内存中逆序后为</span>0065cd1d00000000


<span lang="zh-CN">将这些字段组合在一起，形成合约内容：</span>

ff01020065cd1d00000000


<h4 id=__RefHeading__1770_1702457476>3.6.1<span lang="zh-CN">查看</span>dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6<span lang="zh-CN">当前余额 ：</span> </h4>

xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src\$ ./dacrs-d
-datadir=/home/share/xgc/dacrs\_test getbalance
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6

{

"balance" : 1004.99900000

}



<h4 id = __RefHeading__1772_1702457476> 3.6.2<span lang="zh-CN">提现</span> </h4>
xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src\$ ./dacrs-d
-datadir=/home/share/xgc/dacrs\_test createcontracttx
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6 295-1 0 ff01020065cd1d00000000 100000
0

{

"hash" :
"ce0cf0817e91780b1f7fa1b63ce0af6cbd788024f0c3c15080c2b6748f12e144"

}

xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src\$ ./dacrs-d
-datadir=/home/share/xgc/dacrs\_test setgenerate true 1

{

"msg" : "in mining"

}



<h4 id = __RefHeading__1774_1702457476> 3.6.3 <span lang="zh-CN">再次查看</span>dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6<span lang="zh-CN">当前余额 ，发现已经提现了</span>5<span lang="zh-CN">个币。</span> </h4>

xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src\$ ./dacrs-d
-datadir=/home/share/xgc/dacrs\_test getbalance
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6

{

"balance" : 1009.99800000

}

<h3 id = __RefHeading__1776_1702457476> 3.7 <span lang="zh-CN">脚本出错处理</span> </h3>

<p align="left" style="margin-bottom: 0in; line-height: 100%">
<font face="宋体"><span lang="zh-CN"><font size="4" >假如现在再次提现一次，因为</font></span></font><font face="宋体, serif"><font size="4" >dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >脚本账号下自由金额为</font></span></font><font face="宋体, serif"><font size="4" >0</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >，故提现操作会失败，要查看出错信息，修改</font></span></font><font face="宋体, serif"><font size="4" >Dacrs.conf</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >文件，打开</font></span></font><font face="宋体, serif"><font size="4" >vm</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >调试开关，</font></span></font><font face="宋体, serif"><font size="4" >debug=vm</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >。</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%">
<font face="宋体"><span lang="zh-CN"><font size="4" >排错主要靠</font></span></font><font face="宋体, serif"><font size="4" >LogPrint</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >，</font></span></font><font face="宋体, serif"><font size="4" >assert</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >，</font></span></font><font face="宋体, serif"><font size="4" >error</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >打印出错信息到日志。</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4">xgc@rangershi-MS-7817:/home/share/xgc/dacrs/src$
./dacrs-d -datadir=/home/share/xgc/dacrs_test createcontracttx
dqhbiqjpT67oUX2aTXu3fAcbpvpcJEV4Y6 295-1 0 ff01020065cd1d00000000
100000 0</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >error:
{&quot;code&quot;:-4,&quot;message&quot;:&quot;Error:run-script-error&quot;}</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font size="4" >打开</font></span></font><font face="宋体, serif"><font size="4" >regtest</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >目录下的</font></span></font><font face="宋体, serif"><font size="4" st">vm.log:</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >2016-10-12
02:12:49 [txmempool.cpp:125]vm: tx
hash=69a787e4c7a35f40eff5f696c9f361972a9cb8a7e1e4263e527e689b9bd23e2c
CheckTxInMemPool run contract</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >2016-10-12
02:12:49 [vm/vmrunevn.cpp:125]vm: tx
hash:69a787e4c7a35f40eff5f696c9f361972a9cb8a7e1e4263e527e689b9bd23e2c
fees=100000 fuelrate=100 maxstep:90000</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >2016-10-12
02:12:49 [vm/vmrunevn.cpp:93]vm: CVmScriptRun::intial() LUA</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >2016-10-12
02:12:49 [vm/vmlua.cpp:252]vm: pVmScriptRun=0x7fa006ffb480</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >2016-10-12
02:12:49 [vm/vmlua.cpp:259]vm: luaL_loadbuffer fail:[string
&quot;line&quot;]:375: Account balance is 0.</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >2016-10-12
02:12:49 [vm/vmlua.cpp:262]vm: run step=-1</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体, serif"><font size="4" >2016-10-12
02:12:49 [vm/vmrunevn.cpp:136]vm: CVmScriptRun::run() LUA</font></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font size="4" >其中</font></span></font><font face="宋体, serif"><font size="4" >luaL_loadbuffer
fail:[string &quot;line&quot;]:375: Account balance is
0.</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >提示了脚本</font></span></font><font face="宋体, serif"><font size="4" >375</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >行账户余额为</font></span></font><font face="宋体, serif"><font size="4" >0</font></font><font face="宋体"><span lang="zh-CN"><font size="4" >，从而知道合约为啥会执行不成功。</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><font face="宋体"><span lang="zh-CN"><font size="4" >如果在开发过程中，边写代码边调试，可能写的代码有语法错误，在执行时也会出错，通过上面的方法可以查看出错误行。将错误修正后，要重新注册该脚本，再进行调试。</font></span></font></p>
<p align="left" style="margin-bottom: 0in; line-height: 100%"><br/>

</p>



<h2 id=__RefHeading__1778_1702457476>4部署</h2>  

<span
lang="zh-CN">在局域网测试完毕后，可以先在测试网络再测试一遍，然后部署到主网络。</span>

<span lang="zh-CN">修改配置文件</span>Dacrs.conf<span
lang="zh-CN">。</span>

<span lang="zh-CN">测试网络配置文件修改：</span>
<br/>


<div>
rpcuser=smartcoin <br/>
rpcpassword=admin <br/>
blockminsize=1000<br/>
debug=ERROR<br/>
#debug=vm<br/>
#debug=NOUI<br/>
#debug=TOUI<br/>
debug=INFO<br/>
#debug=net<br/>
#debug=sendtx<br/>
printtoconsole=0<br/>
logtimestamps=1<br/>
logprintfofile=1<br/>
logprintfileline=1<br/>
listen=1<br/>
server=1<br/>
rpcport=18332<br/>
uiport=4246<br/>
#gen=1<br/>
genproclimit=1000000<br/>
logmaxsize=100<br/>
testnet=1

</div>




<span lang="zh-CN">主网络配置文件修改：</span>

<div>
rpcuser=smartcoin <br/>
rpcpassword=admin <br/>
blockminsize=1000<br/>
debug=ERROR<br/>
#debug=vm<br/>
#debug=NOUI<br/>
#debug=TOUI<br/>
debug=INFO<br/>
#debug=net<br/>
#debug=sendtx<br/>
printtoconsole=0<br/>
logtimestamps=1<br/>
logprintfofile=1<br/>
logprintfileline=1<br/>
listen=1<br/>
server=1<br/>
rpcport=18332<br/>
uiport=4246<br/>
#gen=1<br/>
genproclimit=1000000<br/>
logmaxsize=100<br/>
</div>



<span lang="zh-CN">然后像上面那样启动</span>dacrs-d<span
lang="zh-CN">，再使用</span>rpc<span
lang="zh-CN">命令</span>registerapptx<span
lang="zh-CN">注册脚本。</span>



