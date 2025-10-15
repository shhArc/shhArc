function shhInitialize(sd)
	shh.InitializeModule("bob", {})
end

function shhUpdate(u)
	shh.UpdateModule("bob", u, 1)
	shh.UpdateScheduler(u, 1)
end

function shhFinalize()
	shh.FinalizeModule("bob")
end