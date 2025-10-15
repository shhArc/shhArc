function shhInitialize(sd)
	Updater.InitializeModule("bob", {})
end

function shhUpdate(u)
	Updater.UpdateModule("bob", u, 1)
	Updater.UpdateScheduler(u, 1)
end

function shhFinalize()
	Updater.FinalizeModule("bob")
end