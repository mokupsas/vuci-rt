local M = {}

function M.get_time(params)
    local time = os.date("%H:%M:%S")
    params.time = time
    return params
end

return M