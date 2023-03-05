--[[
--
--
--
--]]

--[[ This Function gets called once upon selecting the game mode --]]
function start()
    -- Initialization and backgrounds
    initGameMode("Instinct", 255, 0, 0) -- Name and frame color (r, g, b)
    initBackgrounds(1) -- Number of backgrounds
    --for i = 0, 9, 1 do
    --    initBgIdx(i, "res/bg/gradeMaster1/0"..i..".png")
    --end
    --setBackground(0)
    setPieceTint(255, 255, 255, 255)
    setGravity(5120)

    setDAS(10)
    setARR(1)
    setARE(12)
    setLineARE(8)
    setLockDelay(18)
    setClearSpeed(6)
    setPreview(3)
    setBagRetry(6)
    setGradeType(1)
    setGrade(0)
    setSevenBag(false)
    setHold(true)
    setSonicDrop(true)
    setExtraKicks(true)
    set3D(true)
    setDrawNextBg(true)
    setBig(false)
    setInvis(false)
    setDisplayGrade(false)
    setGhost(false)
    setCreditRoll(false)
    setMaxLevel(1200)
    setLevel(mode, 0)
end

--[[ This Function gets called every frame --]]
function update()
    setSectionLevel(getLevel() + 100)
    if (getSectionLevel() % 100 ~= 0) then
        setSectionLevel(getSectionLevel() - getLevel())
    end
end

function advanceLevel(amount, level, lineCount)

    setLevel(amount + level + lineCount)

    if level >= 1000 then
        setBoneBlocks(true)
        set3D(false)
    else
        setBoneBlocks(false)
        set3D(true)
    end

    if level < 200 then
        setLockDelay(18)
	elseif level < 300 then
        setLockDelay(17)
	elseif level < 500 then
        setLockDelay(15)
	elseif level < 600 then
        setLockDelay(13)
	elseif level < 1100 then
        setLockDelay(12)
	elseif level < 1200 then
        setLockDelay(10)
	elseif level < 1300 then
        setLockDelay(8)
    else
        setLockDelay(8)
    end


    --bgIdx = math.floor(level / 100)
    --setBackground(bgIdx)
end
