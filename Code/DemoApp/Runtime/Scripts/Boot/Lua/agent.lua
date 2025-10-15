function shhMain(sd)
	shh.Trace("boot_script = %", sd["boot_script"])
	ok, agent, vec = Agent("DummyAgent1b")
	shh.SendMsg(agent, 0.0, "", "Dave", Vector(-10,-10,-10))
end

