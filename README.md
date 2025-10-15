# shhArc
SCRIPTED AGENT ENGINE
<img src="images/logo.jpg" alt="Logo" width="1000">
<br><br>
<h2>Overview</h2>
The shhArc engine supports multiple  simulated worlds. 
In order to do this it has a God object that manages the worlds. 
The God object has its own agent system that can be used for user interface agents and world managing agents. 
It also has a virtual machine used for booting the engine, creating the worlds and manipulating them. 
Each world has its own agent system and virtual machine. Agents in different worlds do not know about each other unless informed by God vm/agents.
An agent can have sub processes and these can either be used to execute scripts or as Nodes belonging to the agent.
Agents can communicate with each other via a sandboxed messageing system<br><br>
Nodes are process within and agent and like agents have their own script. The can also comunicate with each other via the sandboxed messaging system 
but only with other nodes within the same agent and to thier owning agent. Nodes also have input and output iterfaces which can be connected to those in 
other nodes via edges to facilitate fast transfer of information.<br<br>
<br><br><br><center><img src="images/intro.jpg"/></center>
The scripting documentation and be found  <a href="https://github.com/shhArc/shhArc/blob/main/Docs/ScriptingManual/">here</a>.
<br><br><br>
<h2>Installation Nodes</h2>
Clone this shhArc repo and the shhThirdParty repo found <a href="https://github.com/shhArc/shhThirdParty">here</a>.
<br><br><br>
Set the following environment variables:<br><br>
SHH_ENGINE = YOURPATH\shhArc\Code<br>
SHH_GCPTR = YOURPATH\shhArc\Code\GCPtr\<br>
SHH_MEMORYMANAGEMENT = YOURPATH\shhArc\Code\MemoryManagement\<br>
SHH_THIRDPARY = YOURPATH\shhThirdParty\<br>
<br><br>
Load the demo Visual Studion project: YOURPATH\shhArc\Code\DemoApp\Win64\DemoApp.sln<br><br>
Set the runtime directory to: YOURPATH\shhArc\Code\DemoApp\Runtime<br><br>
Build, run & enjoy.
<br><br><br>
<h2>Licence</h2>
shhArc is available under the MIT License with a No Modification clause. Except for the files in Code/Config which do not have the No Modification Clause, as you need to edit these to configure shhArc as you want it.
<br><br><br>
<h2>Release Notes</h2>
Current version is Alpha 0.001.<br><br>
Outstanding features to come:<br>
Multi Threading<br>
Persistance<br>
<!--
**shhArc/shhArc** is a ✨ _special_ ✨ repository because its `README.md` (this file) appears on your GitHub profile.

Here are some ideas to get you started:

- 🔭 I’m currently working on ...
- 🌱 I’m currently learning ...
- 👯 I’m looking to collaborate on ...
- 🤔 I’m looking for help with ...
- 💬 Ask me about ...
- 📫 How to reach me: ...
- 😄 Pronouns: ...
- ⚡ Fun fact: ...
-->
