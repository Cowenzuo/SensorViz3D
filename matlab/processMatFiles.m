function processMatFiles(folderPath, factors)
    % 检查文件夹是否存在
    if ~isfolder(folderPath)
        error('目录不存在: %s', folderPath);
    end
    
    % 确保factors为行向量
    factors = factors(:).';
    
    % 获取所有.mat文件
    matFiles = dir(fullfile(folderPath, '*.mat'));
    
    % 遍历每个文件
    for iFile = 1:numel(matFiles)
        fileName = matFiles(iFile).name;
        filePath = fullfile(folderPath, fileName);
        
        % 加载所有变量到结构体
        fileData = load(filePath);
        vars = fieldnames(fileData);
        modified = false;
        
        % 处理每个变量
        for iVar = 1:numel(vars)
            varName = vars{iVar};
            varData = fileData.(varName);
            
            % 检查是否为二维数值矩阵
            if isnumeric(varData) && ismatrix(varData)
                [~, nCols] = size(varData);
                
                % 列数匹配时进行处理
                if nCols == numel(factors)
                    % 列向量化运算
                    fileData.(varName) = varData ./ factors;
                    modified = true;
                end
            end
        end
        
        % 保存修改后的数据
        if modified
            save(filePath, '-struct', 'fileData');
        end
    end
    disp('处理完成！');
end