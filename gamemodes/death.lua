--[[
--
--
--
--]]

--[[ This Function gets called once upon selecting the game mode --]]
function start()
    -- Initialization and backgrounds
    initGameMode("Death 1", 255, 0, 0) -- Name and frame color (r, g, b)
    initBackgrounds(1) -- Number of backgrounds
    --for i = 0, 9, 1 do
    --    initBgIdx(i, "res/bg/gradeMaster1/0"..i..".png")
    --end
    --setBackground(0)
    setPieceTint(150, 150, 150, 255)
    setGravity(5120)

    setDAS(12)
    setARR(1)
    setARE(18)
    setLineARE(14)
    setLockDelay(30)
    setClearSpeed(12)
    setPreview(1)
    setBagRetry(4)
    setGradeType(1)
    setGrade(0)
    setSevenBag(false)
    setHold(false)
    setSonicDrop(true)
    setExtraKicks(false)
    set3D(false)
    setDrawNextBg(true)
    setBig(false)
    setInvis(false)
    setDisplayGrade(false)
    setGhost(false)
    setCreditRoll(false)
    setMaxLevel(999)
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


    --bgIdx = math.floor(level / 100)
    --setBackground(bgIdx)
end
