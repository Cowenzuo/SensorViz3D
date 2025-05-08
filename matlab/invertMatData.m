function invertMatData(folderPath)
    % 检查文件夹有效性
    if ~isfolder(folderPath)
        error('目录不存在: %s', folderPath);
    end
    
    % 获取所有MAT文件
    fileList = dir(fullfile(folderPath, '*.mat'));
    
    % 遍历处理文件
    for i = 1:length(fileList)
        filePath = fullfile(folderPath, fileList(i).name);
        
        try
            % 加载数据
            dataStruct = load(filePath);
            
            % 验证数据存在性
            if ~isfield(dataStruct, 'Datas')
                warning('文件 %s 缺少Datas变量，已跳过', fileList(i).name);
                continue;
            end
            
            % 获取数据矩阵
            datas = dataStruct.Datas;
            
            % 验证数据类型
            if ~isnumeric(datas)
                warning('文件 %s 中Datas不是数值类型，已跳过', fileList(i).name);
                continue;
            end
            
            % 执行取反操作
            dataStruct.Datas = -datas;
            
            % 保存修改（保留其他变量）
            save(filePath, '-struct', 'dataStruct');
            
        catch ME
            warning('文件 %s 处理失败，原因: %s', fileList(i).name, ME.message);
        end
    end
    disp('数据取反处理完成！');
end