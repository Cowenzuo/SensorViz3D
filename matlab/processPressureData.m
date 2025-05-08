function processPressureData(folderPath)
    % 检查文件夹有效性
    if ~isfolder(folderPath)
        error('目录不存在: %s', folderPath);
    end
    
    % 计算固定系数
    D1 = 650;   % 主直径（单位：mm）
    D2 = 360;   % 副直径（单位：mm）
    gravity = 9806.65; % 重力加速度（单位：mm/s²）
    
    % 计算转换系数
    factor1 = pi*(D1/2)^2 / gravity;
    factor2 = (pi*(D1/2)^2 - pi*(D2/2)^2) / gravity;
    
    % 获取所有MAT文件
    fileList = dir(fullfile(folderPath, '*.mat'));
    
    % 遍历处理文件
    for i = 1:length(fileList)
        filePath = fullfile(folderPath, fileList(i).name);
        
        % 加载数据
        dataStruct = load(filePath);
        
        % 验证数据存在性
        if ~isfield(dataStruct, 'Datas')
            warning('文件 %s 缺少Datas变量，已跳过', fileList(i).name);
            continue;
        end
        
        % 获取数据矩阵
        datas = dataStruct.Datas;
        
        % 验证列数
        if size(datas,2) ~= 4
            warning('文件 %s 中Datas列数异常，已跳过', fileList(i).name);
            continue;
        end
        
        % 执行列转换
        datas(:, [1,2]) = datas(:, [1,2]) * factor1;  % 处理第1、2列
        datas(:, [3,4]) = datas(:, [3,4]) * factor2;  % 处理第3、4列
        
        % 计算新列
        newCol5 = datas(:,3) - datas(:,1) ;  % 第5列：P3-P1
        newCol6 = datas(:,4) - datas(:,2);  % 第6列：P4-P2
        
        % 扩展矩阵
        datas = [datas, newCol5, newCol6];
        
        % 更新数据
        dataStruct.Datas = datas;
        
        % 保存修改
        save(filePath, '-struct', 'dataStruct');
    end
    disp('压力数据处理完成！');
end